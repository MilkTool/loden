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
    agpu_device_open_info openInfo;
    memset(&openInfo, 0, sizeof(openInfo));
    switch(windowInfo.subsystem)
    {
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
    case SDL_SYSWM_WINDOWS:
        openInfo.window = (agpu_pointer)windowInfo.info.win.window;
        openInfo.surface = (agpu_pointer)windowInfo.info.win.hdc;
        break;
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
    case SDL_SYSWM_X11:
        openInfo.display = (agpu_pointer)windowInfo.info.x11.display;
        openInfo.window = (agpu_pointer)(uintptr_t)windowInfo.info.x11.window;
        break;
#endif
    default:
        printError("Unsupported window system\n");
        return nullptr;
    }

    openInfo.red_size = 5;
    openInfo.blue_size = 5;
    openInfo.green_size = 5;
    openInfo.alpha_size = 5;
    openInfo.depth_size = 16,
    openInfo.doublebuffer = 1;
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

	// Create the pipeline state manager
	auto pipelineStateManager = std::make_shared<PipelineStateManager> (device);
	auto screenCanvas = AgpuCanvas::create(pipelineStateManager);
	if(!screenCanvas)
	{
		printError("Failed to create the screen canvas\n");
		return nullptr;
	}
	
	// Create the transformation buffer.
	agpu_ref<agpu_buffer> transformationBuffer;
	{
	    agpu_buffer_description desc;
	    desc.size = sizeof(TransformationBlock);
	    desc.usage = AGPU_DYNAMIC;
	    desc.binding = AGPU_UNIFORM_BUFFER;
	    desc.mapping_flags = AGPU_MAP_DYNAMIC_STORAGE_BIT | AGPU_MAP_WRITE_BIT;
	    desc.stride = 0;
	    transformationBuffer = device->createBuffer(&desc, nullptr);
	}

	// Create the shader bindins.
	auto shaderBindings = device->createShaderResourceBinding(0);
    shaderBindings->bindUniformBuffer(0, transformationBuffer.get());
	
	// Create the command list allocator.
	auto allocator = device->createCommandAllocator();
	if(!allocator)
		return nullptr;

	// Create the command list.
	auto commandList = device->createCommandList(allocator, nullptr);
	if(!commandList)
		return nullptr;
	commandList->close();
	
	// Create the window.
	auto window = SystemWindowPtr(new SystemWindow());
	window->setPosition(glm::vec2(0, 0));
	window->setSize(glm::vec2(w, h));
	window->handle = sdlWindow;
	window->device = device;
	window->pipelineStateManager = pipelineStateManager;
	window->screenCanvas = screenCanvas;
	window->transformationBuffer = transformationBuffer;
	window->globalShaderBindings = shaderBindings;
	window->commandAllocator = allocator;
	window->commandList = commandList;
	
	return window;
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
	// Fill the screen canvas.
	screenCanvas->reset();
	drawOn(screenCanvas.get());
	screenCanvas->close();

    // Compute the screen projection matrix
	auto screenWidth = getWidth();
	auto screenHeight = getHeight();
    transformationBlock.projectionMatrix = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -2.0f, 2.0f);

    // Upload the transformation state.
    transformationBuffer->uploadBufferData(0, sizeof(transformationBlock), &transformationBlock);

    // Build the main command list
	commandAllocator->reset();
	commandList->reset(commandAllocator.get(), nullptr);
	commandList->beginFrame(device->getCurrentBackBuffer());

    // Set the viewport
    commandList->setViewport(0, 0, screenWidth, screenHeight);
    commandList->setScissor(0, 0, screenWidth, screenHeight);
    commandList->setClearColor(0, 0, 0, 0);
    commandList->clear(AGPU_COLOR_BUFFER_BIT);
	
	// Use the transformation block.
	commandList->useShaderResources(globalShaderBindings.get());

	// Execute the screen canvas bundle.
	commandList->executeBundle(screenCanvas->getCommandBundle().get());
	
    // Finish the command list
	commandList->endFrame();
	commandList->close();

	// Queue the command list
    auto queue = device->getDefaultCommandQueue();
    queue->addCommandList(commandList.get());

	// Swap the buffers.
	device->swapBuffers();
}

} // End of namespace GUI
} // End of namespace Loden
