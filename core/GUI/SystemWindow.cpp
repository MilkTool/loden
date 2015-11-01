#include <glm/gtc/matrix_transform.hpp>

#include "SDL_syswm.h"
#include "Loden/Printing.hpp"
#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/GUI/AgpuCanvas.hpp"

namespace Loden
{
namespace GUI
{

SystemWindow::SystemWindow()
{
	handle = nullptr;
    frameCount = 3;
    frameIndex = 0;
	setBackgroundColor(glm::vec4(0.0, 0.0, 0.0, 1.0));
}

SystemWindow::~SystemWindow()
{
	SDL_DestroyWindow(handle);
}

bool SystemWindow::isSystemWindow() const
{
	return true;
}

SystemWindow *SystemWindow::getSystemWindow()
{
	return this;
}

glm::vec2 SystemWindow::getAbsolutePosition() const
{
	return glm::vec2();
}

SystemWindowPtr SystemWindow::create(const std::string &title, int w, int h, int x, int y)
{
	// Initialize the video subsystem.
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printError("Failed to initialize SDL2\n");
		return nullptr;
	}

	// Create the sdl window.
	auto sdlWindow = SDL_CreateWindow(title.c_str(), x, y, w, h, SDL_WINDOW_OPENGL);
	if(!sdlWindow)
	{
		printError("Failed to open window\n");
		return nullptr;
	}

    // Get the platform.
    agpu_platform *platform;
    agpuGetPlatforms(1, &platform, nullptr);
    if(!platform)
    {
        printError("Failed to get the AGPU platform\n");
        return nullptr;
    }

    // Get the window info.
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &windowInfo);

    // Open the device
    agpu_swap_chain_create_info swapChainCreateInfo;
    agpu_device_open_info openInfo;
    memset(&openInfo, 0, sizeof(openInfo));
    memset(&swapChainCreateInfo, 0, sizeof(swapChainCreateInfo));
    switch(windowInfo.subsystem)
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    case SDL_SYSWM_WINDOWS:
        openInfo.window = (agpu_pointer)windowInfo.info.win.window;
        swapChainCreateInfo.surface = (agpu_pointer)windowInfo.info.win.hdc;
        break;
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
    case SDL_SYSWM_X11:
        openInfo.display = (agpu_pointer)windowInfo.info.x11.display;
        swapChainCreateInfo.window = (agpu_pointer)(uintptr_t)windowInfo.info.x11.window;
        break;
#endif
    default:
        printError("Unsupported window system\n");
        return nullptr;
    }

    swapChainCreateInfo.colorbuffer_format = AGPU_TEXTURE_FORMAT_R8G8B8A8_UNORM;
    swapChainCreateInfo.depth_stencil_format = AGPU_TEXTURE_FORMAT_D16_UNORM;
    swapChainCreateInfo.width = w;
    swapChainCreateInfo.height = h;
    swapChainCreateInfo.doublebuffer = 1;
#ifdef _DEBUG
    // Use the debug layer when debugging. This is useful for low level backends.
    openInfo.debugLayer = true;
#endif
	agpu_ref<agpu_device> device = platform->openDevice(&openInfo);
	if(!device)
	{
        printError("Failed to open the AGPU device\n");
        return nullptr;
	}

    // Get the main command queue.
    agpu_ref<agpu_command_queue> commandQueue = device->getDefaultCommandQueue();
    if(!commandQueue)
    {
        printError("Failed to get the default AGPU command queue\n");
        return nullptr;
    }

    // Create the swap chain.
    agpu_ref<agpu_swap_chain> swapChain = device->createSwapChain(commandQueue.get(), &swapChainCreateInfo);
    if(!swapChain)
    {
        printError("Failed to create the swap chain\n");
        return nullptr;
    }

    // Create the window.
	auto window = SystemWindowPtr(new SystemWindow());
	window->setPosition(glm::vec2(0, 0));
	window->setSize(glm::vec2(w, h));
	window->handle = sdlWindow;
	window->device = device;
    window->swapChain = swapChain;
    window->commandQueue = commandQueue;

    if(!window->initialize())
        return nullptr;

	return window;
}

bool SystemWindow::initialize()
{
    // Create the pipeline state manager.
    pipelineStateManager = std::make_shared<PipelineStateManager> (device);
    if(!pipelineStateManager->initialize())
    {
        printError("Failed to initialize the pipeline state manager.\n");
        return false;
    }

    // Create the transformation buffer.
    {
        agpu_buffer_description desc;
        desc.size = sizeof(TransformationBlock)*3;
        desc.usage = AGPU_DYNAMIC;
        desc.binding = AGPU_UNIFORM_BUFFER;
        desc.mapping_flags = AGPU_MAP_WRITE_BIT | AGPU_MAP_PERSISTENT_BIT | AGPU_MAP_COHERENT_BIT;
        desc.stride = 0;
        transformationBuffer = device->createBuffer(&desc, nullptr);
        if(!transformationBuffer)
        {
            printError("Failed to create an uniform buffer object.\n");
            return false;
        }

        transformationBlockData = (TransformationBlock*)transformationBuffer->mapBuffer(AGPU_WRITE_ONLY);
        if(!transformationBlockData)
        {
            printError("Failed to map an uniform buffer object.\n");
            return false;
        }
    }

    // Get the gui shader signature.
    shaderSignature = pipelineStateManager->getShaderSignature("GUI");
    if(!shaderSignature)
    {
        printError("Failed to retrieve the GUI shader signature.\n");
        return false;
    }

    // Create the command lists and allocators
    frameCount = 3;
    frameIndex = 0;
    for(int i = 0; i < frameCount; ++i)
    {
        commandAllocators[i] = device->createCommandAllocator();
        if(!commandAllocators[i])
        {
            printError("Failed to create a command allocator.\n");
            return false;
        }

        commandLists[i] = device->createCommandList(commandAllocators[i].get(), nullptr);
    	if(!commandLists[i])
        {
            printError("Failed to create a command list. \n");
            return false;
        }

        commandLists[i]->close();

        frameFences[i] = device->createFence();
        if(!frameFences[i])
        {
            printError("Failed to create a command list. \n");
            return false;
        }

        screenCanvases[i] = AgpuCanvas::create(pipelineStateManager);
        if(!screenCanvases[i])
        {
            printError("Failed to create the screen canvas.\n");
            return false;
        }

        // Create the shader bindings.
        globalShaderBindings[i] = shaderSignature->createShaderResourceBinding(0);
        globalShaderBindings[i]->bindUniformBufferRange(0, transformationBuffer.get(), sizeof(TransformationBlock)*i, sizeof(TransformationBlock));
        if(!globalShaderBindings[i])
        {
            printError("Failed to create GUI shader resource binding\n");
            return false;
        }
    }

    return true;
}

const PipelineStateManagerPtr &SystemWindow::getPipelineStateManager()
{
	return pipelineStateManager;
}

void SystemWindow::pumpEvents()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			{
				KeyboardEvent ev(event.key.keysym.sym, true);
				handleKeyDown(ev);
			}
			break;
		case SDL_KEYUP:
			{
				KeyboardEvent ev(event.key.keysym.sym, false);
				handleKeyUp(ev);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			{
				auto &bev = event.button;
				MouseButtonEvent ev(glm::vec2(bev.x, bev.y), bev.button, true);
				if(mouseCaptureWidget)
				{
					auto newEvent = ev.translatedBy(-mouseCaptureWidget->getAbsolutePosition());
					mouseCaptureWidget->handleMouseButtonDown(newEvent);
				}
				else
				{
					handleMouseButtonDown(ev);
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			{
				auto &bev = event.button;
				MouseButtonEvent ev(glm::vec2(bev.x, bev.y), bev.button, false);
				if(mouseCaptureWidget)
				{
					auto newEvent = ev.translatedBy(-mouseCaptureWidget->getAbsolutePosition());
					mouseCaptureWidget->handleMouseButtonUp(newEvent);
				}
				else
				{
					handleMouseButtonUp(ev);
				}
			}
			break;
		case SDL_MOUSEMOTION:
			{
				auto &bev = event.motion;
				MouseMotionEvent ev(glm::vec2(bev.x, bev.y), glm::vec2(bev.xrel, bev.yrel));
				if(mouseCaptureWidget)
				{
					auto newEvent = ev.translatedBy(-mouseCaptureWidget->getAbsolutePosition());
					mouseCaptureWidget->handleMouseMotion(newEvent);
				}
				else
				{
					handleMouseMotion(ev);
				}
			}
			break;
		case SDL_QUIT:
			{
				Event ev;
				quitEvent(ev);
			}
			break;
		}
	}
}

void SystemWindow::handleKeyDown(KeyboardEvent &event)
{
	keyDownEvent(event);
	if(!event.wasHandled() && focusedWidget && focusedWidget.get() != this)
		focusedWidget->handleKeyDown(event);
}

void SystemWindow::handleKeyUp(KeyboardEvent &event)
{
	keyUpEvent(event);
	if(!event.wasHandled() && focusedWidget && focusedWidget.get() != this)
		focusedWidget->handleKeyUp(event);
}

void SystemWindow::setKeyboardFocusWidget(const WidgetPtr &newKeyboardFocus)
{
	auto oldFocus = focusedWidget;
	focusedWidget = newKeyboardFocus;

	FocusEvent event(oldFocus, newKeyboardFocus);
	if(oldFocus)
		oldFocus->handleLostFocus(event);
	if(newKeyboardFocus)
		newKeyboardFocus->handleGotFocus(event);
}

void SystemWindow::setMouseFocusWidget(const WidgetPtr &newMouseFocus)
{
	auto oldFocus = mouseOverWidget;
	mouseOverWidget = newMouseFocus;

	MouseFocusEvent event(oldFocus, newMouseFocus);
	if(oldFocus)
		oldFocus->handleMouseLeave(event);
	if(newMouseFocus)
		newMouseFocus->handleMouseEnter(event);
}

void SystemWindow::setMouseCaptureWidget(const WidgetPtr &widget)
{
	mouseCaptureWidget = widget;
	SDL_SetWindowGrab(handle, mouseCaptureWidget.get() ? SDL_TRUE : SDL_FALSE);
}

void SystemWindow::renderScreen()
{
    //printf("Render frame %d\n", frameIndex);
    auto& commandAllocator = commandAllocators[frameIndex];
    auto& commandList = commandLists[frameIndex];
    auto& screenCanvas = screenCanvases[frameIndex];

    // Ensure the frame data is not pending.
    frameFences[frameIndex]->waitOnClient();

	// Fill the screen canvas.
	screenCanvas->reset();
	drawOn(screenCanvas.get());
	screenCanvas->close();

    // Compute the screen projection matrix
    auto &transformationBlock = transformationBlockData[frameIndex];
	auto screenWidth = getWidth();
	auto screenHeight = getHeight();
    transformationBlock.projectionMatrix = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -2.0f, 2.0f);

    // Build the main command list
    agpu_ref<agpu_framebuffer> backBuffer = swapChain->getCurrentBackBuffer();
	commandAllocator->reset();
	commandList->reset(commandAllocator.get(), nullptr);
    commandList->setShaderSignature(shaderSignature.get());
	commandList->beginFrame(backBuffer.get());

    // Set the viewport
    commandList->setViewport(0, 0, screenWidth, screenHeight);
    commandList->setScissor(0, 0, screenWidth, screenHeight);
    commandList->setClearColor(0, 0, 0, 0);
    commandList->clear(AGPU_COLOR_BUFFER_BIT);

	// Use the transformation block.
	commandList->useShaderResources(globalShaderBindings[frameIndex].get());

	// Execute the screen canvas bundle.
	commandList->executeBundle(screenCanvas->getCommandBundle().get());

    // Finish the command list
	commandList->endFrame();
	commandList->close();

	// Queue the command list
    commandQueue->addCommandList(commandList.get());

	// Swap the buffers.
	swapChain->swapBuffers();
    commandQueue->signalFence(frameFences[frameIndex].get());

    frameIndex = (frameIndex + 1) % frameCount;
}

} // End of namespace GUI
} // End of namespace Loden
