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
	qreal radius = (m_size.width()*m_size.width() + m_size.height()*m_size.height()) / (4.0 * m_size.height());
	QPointF center(m_size.width()/2.0, m_size.height() + radius - m_size.height()/2.0);
	static const qreal radiusInc = 10.0;
	QRadialGradient gradient(center, radius + radiusInc, center);
	gradient.setColorAt((radius - m_size.height()/2.0)/(radius + radiusInc), QColor(255, 255, 255, static_cast<int>(150*m_highlightIntensity)));
	gradient.setColorAt(1, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());
}

void Applet::animateHighlight()
{
	static const qreal highlightAnimationSpeed = 0.1;
	if(isHighlighted())
	{
		m_highlightIntensity += highlightAnimationSpeed;
		if(m_highlightIntensity > 1.0)
			m_highlightIntensity = 1.0;
		else
			QTimer::singleShot(20, this, SLOT(animateHighlight()));
	}
	else
	{
		m_highlightIntensity -= highlightAnimationSpeed;
		if(m_highlightIntensity < 0.0)
			m_highlightIntensity = 0.0;
		else
			QTimer::singleShot(20, this, SLOT(animateHighlight()));
	}
	update();
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

bool Applet::isHighlighted()
{
	return isUnderMouse();
}

void Applet::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	animateHighlight();
}

void Applet::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	animateHighlight();
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
