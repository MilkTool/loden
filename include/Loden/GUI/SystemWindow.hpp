#ifndef LODEN_GUI_SYSTEM_WINDOW_HPP
#define LODEN_GUI_SYSTEM_WINDOW_HPP

#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/Engine.hpp"
#include "Loden/TransformationBlock.hpp"
#include "SDL.h"
#include <string>

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(SystemWindow);
LODEN_DECLARE_CLASS(AgpuCanvas);

/**
 * The system window.
 */
class LODEN_CORE_EXPORT SystemWindow: public ContainerWidget
{
public:
	~SystemWindow();

	static SystemWindowPtr create(const EnginePtr &engine, const std::string &title, int w, int h, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED);

	virtual bool isSystemWindow() const;
	virtual SystemWindow *getSystemWindow();
    virtual EnginePtr getEngine();

	virtual glm::vec2 getAbsolutePosition() const;

	void setKeyboardFocusWidget(const WidgetPtr &newKeyboardFocus);
	void setMouseFocusWidget(const WidgetPtr &newMouseFocus);
	void setMouseCaptureWidget(const WidgetPtr &widget);

	void pumpEvents();
	void renderScreen();

	virtual void handleKeyDown(KeyboardEvent &event);
	virtual void handleKeyUp(KeyboardEvent &event);

    void setTitle(const std::string &title);

public:
	EventSocket<Event> quitEvent;

private:
	SystemWindow();
    bool initialize();

	SDL_Window *handle;
	WidgetPtr focusedWidget;
	WidgetPtr mouseOverWidget;
	WidgetPtr mouseCaptureWidget;
    EnginePtr engine;

    agpu_device_ref device;
    agpu_command_queue_ref commandQueue;
    agpu_swap_chain_ref swapChain;
    agpu_shader_signature_ref shaderSignature;

	agpu_buffer_ref transformationBuffer;
    uint8_t *transformationBlockData;

	agpu_ref<agpu_command_allocator> commandAllocators[3];
	agpu_ref<agpu_command_list> commandLists[3];
    agpu_ref<agpu_fence> frameFences[3];
    agpu_ref<agpu_shader_resource_binding> globalShaderBindings[3];
    AgpuCanvasPtr screenCanvases[3];
    int frameCount;
    int frameIndex;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_SYSTEM_WINDOW_HPP
