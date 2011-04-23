#include "applet.h"

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include "panelwindow.h"

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow), m_highlightIntensity(0.0)
{
	setZValue(-1.0);
	setParentItem(m_panelWindow->panelItem());
}

Applet::~Applet()
{
}

bool Applet::init()
{
	m_panelWindow->updateLayout();
	return true;
}

void Applet::setPosition(const QPoint& position)
{
	m_position = position;
	setPos(m_position);
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
		setAcceptsHoverEvents(true);
		setAcceptedMouseButtons(Qt::LeftButton);
	}
	else
	{
		setAcceptsHoverEvents(false);
		setAcceptedMouseButtons(Qt::NoButton);
	}
}

QRectF Applet::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width(), m_size.height());
}

void Applet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(QColor(255, 255, 255, static_cast<int>(40*m_highlightIntensity))));
	painter->drawRect(boundingRect());
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

void Applet::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	m_highlightIntensity = 1.0;
	update();
}

void Applet::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	m_highlightIntensity = 0.0;
	update();
}

void Applet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void Applet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
	{
		// FIXME: Workaround.
		// For some weird reason, if clicked() function is called directly, and menu is opened,
		// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
		// Probably somehow related to taking a mouse grab when one is already active.
		QTimer::singleShot(1, this, SLOT(clicked()));
	}
}
