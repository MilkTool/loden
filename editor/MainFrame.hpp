#ifndef LODEN_LED_MAIN_FRAME_HPP
#define LODEN_LED_MAIN_FRAME_HPP

#include "Loden/Object.hpp"

namespace Loden
{
namespace LevelEditor
{

LODEN_DECLARE_CLASS(MainFrame);
LODEN_DECLARE_CLASS(LevelEditor);

/**
 * Main frame
 */
class MainFrame : public Object
{
    MainFrame(LevelEditor *editor);
public:
    ~MainFrame();

    static MainFramePtr create(LevelEditor *editor);

    bool initialize();

private:
    bool buildGui();

    LevelEditor *editor;
};

} // End of namespace LevelEditor
} // End of namespace MainFrame

#endif //LODEN_LED_MAIN_FRAME_HPP
