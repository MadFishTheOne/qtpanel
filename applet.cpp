#include "applet.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include "panelwindow.h"

AppletGraphicsItem::AppletGraphicsItem(Applet* applet)
	: m_applet(applet), m_highlightIntensity(0.0)
{
	setZValue(-1.0);
	setAcceptsHoverEvents(true);
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

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
	m_appletItem = new AppletGraphicsItem(this);
	m_panelWindow->scene()->addItem(m_appletItem);
}

Applet::~Applet()
{
	m_panelWindow->scene()->removeItem(m_appletItem);
	delete m_appletItem;
}

bool Applet::init()
{
	m_panelWindow->updateLayout();
	return true;
}

void Applet::setPosition(const QPoint& position)
{
	m_appletItem->setPos(position);
}

void Applet::setSize(const QSize& size)
{
	m_size = size;
	layoutChanged();
}

void Applet::layoutChanged()
{
}
