#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/GUI/Button.hpp"
#include "Loden/Application.hpp"
#include "Loden/Engine.hpp"

using namespace Loden;

class Sample2: public Application
{
protected:
	virtual bool initialize();
	virtual bool run();
	virtual bool shutdown();
	
	virtual void pumpEvents();
	virtual void mainLoopUpdateStep(float updateDelta);
	virtual void mainLoopRenderStep();

private:
	GUI::SystemWindowPtr screen;
};

APPLICATION_ENTRY_POINT(Sample2);

bool Sample2::initialize()
{
    // Create thew system window
	screen = GUI::SystemWindow::create(engine, "Sample2", 640, 480);
    if (!screen)
        return false;
	
	auto button = GUI::Button::create("Click Me", glm::vec2(60, 20), glm::vec2(60, 60));
	screen->addChild(button);
	
	screen->quitEvent += [this](GUI::Event &ev) {
		mainLoopQuit();
	};
	
	screen->keyDownEvent += [this](GUI::KeyboardEvent &ev) {
		switch(ev.getSymbol())
		{
		case SDLK_ESCAPE:
			mainLoopQuit();
			ev.setHandled();
			break;
		}
	};
	
	return true;
}

bool Sample2::run()
{
	return enterMainLoop(1.0f/60.0f);
}

bool Sample2::shutdown()
{
	return true;
}

void Sample2::pumpEvents()
{
	screen->pumpEvents();
}

void Sample2::mainLoopUpdateStep(float updateDelta)
{
}

void Sample2::mainLoopRenderStep()
{
	screen->renderScreen();
}
