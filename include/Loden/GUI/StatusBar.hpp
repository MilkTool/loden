#ifndef LODEN_GUI_STATUS_BAR_HPP
#define LODEN_GUI_STATUS_BAR_HPP

#include "Loden/GUI/Widget.hpp"

namespace Loden
{
namespace GUI
{

LODEN_DECLARE_CLASS(StatusBar);

/**
* Status bar
*/
class LODEN_CORE_EXPORT StatusBar : public ObjectSubclass<StatusBar, Widget>
{
    LODEN_OBJECT_TYPE(StatusBar);
public:
    StatusBar(const SystemWindowPtr &systemWindow = nullptr);
    ~StatusBar();

    static StatusBarPtr create(const SystemWindowPtr &systemWindow);

    virtual glm::vec2 getMinimalSize();

    virtual void drawContentOn(Canvas *canvas);

private:

};
} // End of namespace GUI
} // End of namespace Loden

#endif //LODEN_GUI_STATUS_BAR_HPP
