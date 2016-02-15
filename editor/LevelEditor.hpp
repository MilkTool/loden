#ifndef LODEN_LEVEL_EDITOR_HPP
#define LODEN_LEVEL_EDITOR_HPP

#include "Loden/GUI/SystemWindow.hpp"
#include "Loden/Application.hpp"
#include "Loden/Engine.hpp"

namespace Loden
{
namespace LevelEditor
{
LODEN_DECLARE_CLASS(LevelEditor)
LODEN_DECLARE_CLASS(MainFrame)

class LevelEditor : public Application
{
public:
    const GUI::SystemWindowPtr &getScreen()
    {
        return screen;
    }

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
    MainFramePtr mainFrame;
};

} // End of namespace LevelEditor
} // End of namespace LevelEditor

#endif //LODEN_LEVEL_EDITOR_HPP
