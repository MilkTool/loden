#include "MainFrame.hpp"
#include "LevelEditor.hpp"
#include "Loden/GUI/MenuBar.hpp"
#include "Loden/GUI/StatusBar.hpp"
#include "Loden/GUI/Label.hpp"
#include "Loden/GUI/TextInput.hpp"
#include "Loden/GUI/DockingLayout.hpp"

namespace Loden
{
namespace LevelEditor
{

MainFrame::MainFrame(LevelEditor *editor)
    : editor(editor)
{
}

MainFrame::~MainFrame()
{

}

MainFramePtr MainFrame::create(LevelEditor *editor)
{
    MainFramePtr mainFrame(new MainFrame(editor));
    if (!mainFrame->initialize())
        return nullptr;
    return mainFrame;
}

bool MainFrame::initialize()
{
    if (!buildGui())
        return false;
    return true;
}

bool MainFrame::buildGui()
{
    auto &screen = editor->getScreen();
        screen->quitEvent += [this](GUI::Event &ev) {
        editor->mainLoopQuit();
    };

    screen->keyDownEvent += [this](GUI::KeyboardEvent &ev) {
        switch (ev.getSymbol())
        {
        case SDLK_ESCAPE:
            editor->mainLoopQuit();
            ev.setHandled();
            break;
        }
    };

    // Create the menu bar
    auto menuBar = GUI::MenuBar::create(screen);
    screen->addChild(menuBar);

    // Create the menu
    auto fileMenu = GUI::Menu::create(screen);
    fileMenu->addAction("Exit", [this](GUI::Event &ev) {
        editor->mainLoopQuit();
    });
    menuBar->addMenu("File", fileMenu);

    auto editMenu = GUI::Menu::create(screen);
    editMenu->addAction("Cut", [this](GUI::Event &ev) {
    });
    editMenu->addAction("Copy", [this](GUI::Event &ev) {
    });
    editMenu->addAction("Paste", [this](GUI::Event &ev) {
    });
    editMenu->addAction("Delete", [this](GUI::Event &ev) {
    });

    menuBar->addMenu("Edit", editMenu);

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
    return true;
}


} // End of namespace LevelEditor
} // End of namespace Loden
