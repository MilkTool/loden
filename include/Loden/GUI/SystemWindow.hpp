#ifndef LODEN_GUI_SYSTEM_WINDOW_HPP
#define LODEN_GUI_SYSTEM_WINDOW_HPP

#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/Engine.hpp"
#include "Loden/TransformationBlock.hpp"
#include "SDL.h"
#include <string>
#include <set>

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
    LODEN_WIDGET_TYPE(SystemWindow, ContainerWidget);
public:
	~SystemWindow();

	static SystemWindowPtr create(const EnginePtr &engine, const std::string &title, int w, int h, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED);

	virtual bool isSystemWindow() const;
    virtual EnginePtr getEngine();

	virtual glm::vec2 getAbsolutePosition() const;

	void setKeyboardFocusWidget(const WidgetPtr &newKeyboardFocus);
	void setMouseFocusWidget(const WidgetPtr &newMouseFocus);
	void setMouseCaptureWidget(const WidgetPtr &widget);

	void pumpEvents();
	void renderScreen();

    virtual void handleMouseButtonDown(MouseButtonEvent &event) override;

	virtual void handleKeyDown(KeyboardEvent &event) override;
	virtual void handleKeyUp(KeyboardEvent &event) override;

    void setTitle(const std::string &title);

    void activatePopUp(const WidgetPtr &popup, const WidgetPtr &popupGroup);
    void killPopUp(const WidgetPtr &popup, const WidgetPtr &popupGroup);
    virtual void killAllPopUps(const WidgetPtr &popupGroup) override;

    bool hasMultisampling() const;

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

    agpu_framebuffer_ref multisampleFramebuffers[3];
    agpu_texture_ref multisampleColorbuffers[3];
	agpu_command_allocator_ref commandAllocators[3];
	agpu_command_list_ref commandLists[3];
    agpu_fence_ref frameFences[3];
    agpu_shader_resource_binding_ref globalShaderBindings[3];
    AgpuCanvasPtr screenCanvases[3];
    int frameCount;
    int frameIndex;

    std::set<WidgetPtr> popups;
    WidgetPtr currentPopUpGroup;
    unsigned int sampleCount;
    unsigned int sampleQuality;
};

} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_SYSTEM_WINDOW_HPP
