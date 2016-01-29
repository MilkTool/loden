#include "Loden/Printing.hpp"
#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/GUI/AgpuCanvas.hpp"
#include "Loden/Matrices.hpp"
#include "SDL_syswm.h"

namespace Loden
{
namespace GUI
{

SystemWindow::SystemWindow()
    : ContainerWidget(nullptr)
{
	handle = nullptr;
    frameCount = 3;
    frameIndex = 0;
	setBackgroundColor(glm::vec4(0.0, 0.0, 0.0, 1.0));
    setAutoLayout(true);
}

SystemWindow::~SystemWindow()
{
	SDL_DestroyWindow(handle);
}

bool SystemWindow::isSystemWindow() const
{
	return true;
}

EnginePtr SystemWindow::getEngine()
{
    return engine;
}

glm::vec2 SystemWindow::getAbsolutePosition() const
{
	return glm::vec2();
}

SystemWindowPtr SystemWindow::create(const EnginePtr &engine, const std::string &title, int w, int h, int x, int y)
{
	// Initialize the video subsystem.
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printError("Failed to initialize SDL2\n");
		return nullptr;
	}

    int flags = 0;
#ifndef _WIN32
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    flags |= SDL_WINDOW_OPENGL;
#endif

	// Create the sdl window.
	auto sdlWindow = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
	if(!sdlWindow)
	{
		printError("Failed to open window\n");
		return nullptr;
	}

    // Get the window info.
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &windowInfo);

    // Get the device.
    auto &device = engine->getAgpuDevice();

    // Create the swap chain
    agpu_swap_chain_create_info swapChainCreateInfo;
    memset(&swapChainCreateInfo, 0, sizeof(swapChainCreateInfo));
    switch(windowInfo.subsystem)
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    case SDL_SYSWM_WINDOWS:
        swapChainCreateInfo.window = (agpu_pointer)windowInfo.info.win.window;
        swapChainCreateInfo.surface = (agpu_pointer)windowInfo.info.win.hdc;
        break;
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
    case SDL_SYSWM_X11:
        swapChainCreateInfo.window = (agpu_pointer)(uintptr_t)windowInfo.info.x11.window;
        break;
#endif
    default:
        printError("Unsupported window system\n");
        return nullptr;
    }

    swapChainCreateInfo.colorbuffer_format = AGPU_TEXTURE_FORMAT_R8G8B8A8_UNORM;
    swapChainCreateInfo.depth_stencil_format = AGPU_TEXTURE_FORMAT_D24_UNORM_S8_UINT;
    swapChainCreateInfo.width = w;
    swapChainCreateInfo.height = h;
    swapChainCreateInfo.doublebuffer = 1;


    // Create the swap chain.
    agpu_ref<agpu_swap_chain> swapChain = device->createSwapChain(engine->getGraphicsCommandQueue().get(), &swapChainCreateInfo);
    if(!swapChain)
    {
        printError("Failed to create the swap chain\n");
        return nullptr;
    }

    // Create the window.
	auto window = SystemWindowPtr(new SystemWindow());
    window->setSystemWindow(window);
	window->setPosition(glm::vec2(0, 0));
	window->setSize(glm::vec2(w, h));
	window->handle = sdlWindow;
    window->engine = engine;
	window->device = device;
    window->swapChain = swapChain;
    window->commandQueue = engine->getGraphicsCommandQueue();

    if(!window->initialize())
        return nullptr;

	return window;
}

bool SystemWindow::initialize()
{
    // Create the transformation buffer.
    {
        agpu_buffer_description desc;
        desc.size = TransformationBlock_AlignedSize*3;
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

        transformationBlockData = (uint8_t*)transformationBuffer->mapBuffer(AGPU_WRITE_ONLY);
        if(!transformationBlockData)
        {
            printError("Failed to map an uniform buffer object.\n");
            return false;
        }
    }

    // Get the gui shader signature.
    auto &pipelineStateManager = engine->getPipelineStateManager();
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
        commandAllocators[i] = device->createCommandAllocator(AGPU_COMMAND_LIST_TYPE_DIRECT);
        if(!commandAllocators[i])
        {
            printError("Failed to create a command allocator.\n");
            return false;
        }

        commandLists[i] = device->createCommandList(AGPU_COMMAND_LIST_TYPE_DIRECT, commandAllocators[i].get(), nullptr);
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
        globalShaderBindings[i]->bindUniformBufferRange(0, transformationBuffer.get(), TransformationBlock_AlignedSize*i, TransformationBlock_AlignedSize);
        if(!globalShaderBindings[i])
        {
            printError("Failed to create GUI shader resource binding\n");
            return false;
        }
    }

    return true;
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
    auto transformationBlock = reinterpret_cast<TransformationBlock*> (transformationBlockData + TransformationBlock_AlignedSize*frameIndex);
	int screenWidth = (int)ceil(getWidth());
	int screenHeight = (int)ceil(getHeight());
    transformationBlock->projectionMatrix = orthographicMatrix(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -2.0f, 2.0f);
    transformationBlock->modelMatrix = glm::mat4();
    transformationBlock->viewMatrix = glm::mat4();

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
    commandList->setClearDepth(1);
    commandList->setClearStencil(0);
    commandList->clear(AGPU_COLOR_BUFFER_BIT | AGPU_DEPTH_BUFFER_BIT | AGPU_STENCIL_BUFFER_BIT);

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

void SystemWindow::setTitle(const std::string &title)
{
    SDL_SetWindowTitle(handle, title.c_str());
}

} // End of namespace GUI
} // End of namespace Loden
