#include "applet.h"

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include "panelwindow.h"

AppletGraphicsItem::AppletGraphicsItem(Applet* applet)
	: m_applet(applet), m_highlightIntensity(0.0)
{
	setZValue(-1.0);
}

AppletGraphicsItem::~AppletGraphicsItem()
{
}

QRectF AppletGraphicsItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_applet->size().width(), m_applet->size().height());
}

void AppletGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(QColor(255, 255, 255, static_cast<int>(40*m_highlightIntensity))));
	painter->drawRect(boundingRect());
}

void AppletGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	m_highlightIntensity = 1.0;
	update();
}

void AppletGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	m_highlightIntensity = 0.0;
	update();
}

void AppletGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void AppletGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
	{
		// FIXME: Workaround.
		// For some weird reason, if clicked() function is called directly, and menu is opened,
		// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
		// Probably somehow related to taking a mouse grab when one is already active.
		QTimer::singleShot(1, m_applet, SLOT(clicked()));
	}
}

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
	m_appletItem = new AppletGraphicsItem(this);
	m_appletItem->setParentItem(m_panelWindow->panelItem());
}

Applet::~Applet()
{
	delete m_appletItem;
}

bool Applet::init()
{
	m_panelWindow->updateLayout();
	return true;
}

void Applet::setPosition(const QPoint& position)
{
	m_position = position;
	m_appletItem->setPos(m_position);
}

void Applet::setSize(const QSize& size)
{
	m_size = size;
	layoutChanged();
}

void Applet::setInteractive(bool interactive)
{
	if(interactive)
	{
		m_appletItem->setAcceptsHoverEvents(true);
		m_appletItem->setAcceptedMouseButtons(Qt::LeftButton);
	}
	else
	{
		m_appletItem->setAcceptsHoverEvents(false);
		m_appletItem->setAcceptedMouseButtons(Qt::NoButton);
	}
}

void Applet::clicked()
{
}

void Applet::layoutChanged()
{
}

QPoint Applet::localToScreen(const QPoint& point)
{
	return m_panelWindow->pos() + m_position + point;
}
