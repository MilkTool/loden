#ifndef LODEN_GUI_SYSTEM_WINDOW_HPP
#define LODEN_GUI_SYSTEM_WINDOW_HPP

#include <string>
#include "AGPU/agpu.hpp"
#include "SDL.h"
#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/PipelineStateManager.hpp"
#include "Loden/TransformationBlock.hpp"

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

	static SystemWindowPtr create(const std::string &title, int w, int h, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED);

	virtual bool isSystemWindow() const;
	virtual SystemWindow *getSystemWindow();
	virtual glm::vec2 getAbsolutePosition() const;

	void setKeyboardFocusWidget(const WidgetPtr &newKeyboardFocus);
	void setMouseFocusWidget(const WidgetPtr &newMouseFocus);
	void setMouseCaptureWidget(const WidgetPtr &widget);

	const PipelineStateManagerPtr &getPipelineStateManager();

	void pumpEvents();
	void renderScreen();

	virtual void handleKeyDown(KeyboardEvent &event);
	virtual void handleKeyUp(KeyboardEvent &event);

public:
	EventSocket<Event> quitEvent;

private:
	SystemWindow();

	SDL_Window *handle;
	WidgetPtr focusedWidget;
	WidgetPtr mouseOverWidget;
	WidgetPtr mouseCaptureWidget;

	agpu_ref<agpu_device> device;
    agpu_ref<agpu_swap_chain> swapChain;
	PipelineStateManagerPtr pipelineStateManager;
	AgpuCanvasPtr screenCanvas;

	TransformationBlock transformationBlock;
	agpu_ref<agpu_buffer> transformationBuffer;
	agpu_ref<agpu_shader_resource_binding> globalShaderBindings;

	agpu_ref<agpu_command_allocator> commandAllocator;
	agpu_ref<agpu_command_list> commandList;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_SYSTEM_WINDOW_HPP
