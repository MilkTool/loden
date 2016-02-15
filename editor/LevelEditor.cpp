#include "LevelEditor.hpp"
#include "MainFrame.hpp"

namespace Loden
{
namespace LevelEditor
{

bool LevelEditor::initialize()
{
    // Create the system window
    screen = GUI::SystemWindow::create(engine, "Loden Level Editor", 1024, 768);
    if (!screen)
        return false;

    mainFrame = MainFrame::create(this);

    return true;
}

bool LevelEditor::run()
{
    return enterMainLoop(1.0f / 60.0f);
}

bool LevelEditor::shutdown()
{
    return Application::shutdown();
}

void LevelEditor::pumpEvents()
{
    screen->pumpEvents();
}

void LevelEditor::mainLoopUpdateStep(float updateDelta)
{
}

void LevelEditor::mainLoopRenderStep()
{
    screen->renderScreen();
}

void LevelEditor::updateFpsDisplay(float fps)
{
    char buffer[256];
    sprintf(buffer, "Loden Level Editor - %03.2f", fps);
    screen->setTitle(buffer);
}

} // End of namespace LevelEditor
} // End of namespace Loden

APPLICATION_ENTRY_POINT(Loden::LevelEditor::LevelEditor);