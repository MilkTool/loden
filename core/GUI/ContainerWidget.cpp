#include "Loden/GUI/ContainerWidget.hpp"
#include "Loden/GUI/Layout.hpp"

namespace Loden
{
namespace GUI
{
ContainerWidget::ContainerWidget(const SystemWindowPtr &systemWindow)
    : BaseType(systemWindow)
{
    autoLayout = false;
}

ContainerWidget::~ContainerWidget()
{
}

size_t ContainerWidget::getNumberOfChilds()
{
	return children.size();
}

void ContainerWidget::addChild(const WidgetPtr& child)
{
	auto oldParent = child->getParent();
	if(oldParent)
		oldParent->removeChild(child);
		
	children.push_back(child);
	child->setParent(shared_from_this());
}

void ContainerWidget::removeChild(const WidgetPtr& child)
{
	for(size_t i = 0; i < children.size(); ++i)
	{
		if(children[i] == child)
		{
			children.erase(children.begin() + i);
			return;
		}
	}
}

WidgetPtr ContainerWidget::findChildAtPoint(const glm::vec2 &position)
{
	auto it = children.rbegin();
	for(; it != children.rend(); ++it)
	{
		auto &child = *it;
		if(child->getRectangle().containsPoint(position))
			return child;
	}
	
	return nullptr;
}

void ContainerWidget::drawOn(Canvas *canvas)
{
	canvas->withTranslation(getPosition(), [&] {
		// Draw the content
		drawContentOn(canvas);
		
		// Draw the children
		drawChildrenOn(canvas);
	});
}

void ContainerWidget::drawChildrenOn(Canvas *canvas)
{
	for(auto &child : children)
		child->drawOn(canvas);
}

void ContainerWidget::handleMouseButtonDown(MouseButtonEvent &event)
{
	auto child = findChildAtPoint(event.getPosition());
	if(child)
	{
		auto newEvent = event.translatedBy(-child->getPosition());
		child->handleMouseButtonDown(newEvent);
		event.setHandled(newEvent.wasHandled());
	}

	if(!child)
		setMouseOverHere();
		
	if(!event.wasHandled())
		mouseButtonDownEvent(event);
}

void ContainerWidget::handleMouseButtonUp(MouseButtonEvent &event)
{
	auto child = findChildAtPoint(event.getPosition());
	if(child)
	{
		auto newEvent = event.translatedBy(-child->getPosition());
		child->handleMouseButtonUp(newEvent);
		event.setHandled(newEvent.wasHandled());
	}

	if(!child)
		setMouseOverHere();

	if(!event.wasHandled())
		mouseButtonUpEvent(event);
}

void ContainerWidget::handleMouseMotion(MouseMotionEvent &event)
{
	auto child = findChildAtPoint(event.getPosition());
	if(child)
	{
		auto newEvent = event.translatedBy(-child->getPosition());
		child->handleMouseMotion(newEvent);
		event.setHandled(newEvent.wasHandled());
	}
	
	if(!child)
		setMouseOverHere();

	if(!event.wasHandled())
		mouseMotionEvent(event);
}

const LayoutPtr &ContainerWidget::getLayout() const
{
    return layout;
}

void ContainerWidget::setLayout(const LayoutPtr &newLayout)
{
    layout = newLayout;
    if (isAutoLayout())
        updateLayout();
}

bool ContainerWidget::isAutoLayout()
{
    return autoLayout;
}

void ContainerWidget::setAutoLayout(bool newAutoLayout)
{
    autoLayout = newAutoLayout;
}

void ContainerWidget::fitLayout()
{
    if (layout)
        layout->fit(this);
}

void ContainerWidget::updateLayout()
{
    if (layout)
        layout->update(this, getLocalRectangle());
}

} // End of namespace GUI
} // End of namespace Loden
