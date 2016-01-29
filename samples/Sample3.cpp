#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/GUI/MenuBar.hpp"
#include "Loden/GUI/StatusBar.hpp"
#include "Loden/GUI/Label.hpp"
#include "Loden/GUI/DockingLayout.hpp"
#include "Loden/Application.hpp"
#include "Loden/Engine.hpp"

using namespace Loden;

class Sample3: public Application
{
protected:
	virtual bool initialize();
	virtual bool run();
	virtual bool shutdown();

	virtual void pumpEvents();
	virtual void mainLoopUpdateStep(float updateDelta);
	virtual void mainLoopRenderStep();
    virtual void updateFpsDisplay(float fps);

private:
	GUI::SystemWindowPtr screen;
};

APPLICATION_ENTRY_POINT(Sample3);

bool Sample3::initialize()
{
    // Create the system window
	screen = GUI::SystemWindow::create(engine, "Sample3", 640, 480);
    if (!screen)
        return false;

    // Create the menu bar
    auto menuBar = GUI::MenuBar::create(screen);
    screen->addChild(menuBar);

    // Create the menu
    auto fileMenu = GUI::Menu::create(screen);
    fileMenu->addItem("Exit", [this](GUI::Event &ev) {
        mainLoopQuit();
    });
    menuBar->addMenu("File", fileMenu);

    // Create a label
	auto label = GUI::Label::create(screen, "Hello World");
	screen->addChild(label);

    // Create the status bar
    auto statusBar = GUI::StatusBar::create(screen);
    screen->addChild(statusBar);

    // Create the layout
    auto layout = std::make_shared<GUI::VerticalBoxLayout>();
    layout->addWidget(menuBar, 0, 0, GUI::CellLayoutFlags::Expand);
    layout->addWidget(label, 1, 0, GUI::CellLayoutFlags::AlignCenter);
    layout->addWidget(statusBar, 0, 0, GUI::CellLayoutFlags::Expand);
    screen->setLayout(layout);

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

bool Sample3::run()
{
	return enterMainLoop(1.0f/60.0f);
}

bool Sample3::shutdown()
{
	return Application::shutdown();
}

void Sample3::pumpEvents()
{
	screen->pumpEvents();
}

void Sample3::mainLoopUpdateStep(float updateDelta)
{
}

void Sample3::mainLoopRenderStep()
{
	screen->renderScreen();
}

void Sample3::updateFpsDisplay(float fps)
{
    char buffer[256];
    sprintf(buffer, "Sample 2 - %03.2f", fps);
    screen->setTitle(buffer);
}
