#include "applet.h"

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include "panelwindow.h"
#include "animationutils.h"

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow), m_highlightIntensity(0.0), m_interactive(false)
{
	setZValue(-1.0);
	setAcceptedMouseButtons(Qt::RightButton);
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
	m_interactive = interactive;

	if(m_interactive)
	{
		setAcceptHoverEvents(true);
		setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);
	}
	else
	{
		setAcceptHoverEvents(false);
		setAcceptedMouseButtons(Qt::RightButton);
	}
}

QRectF Applet::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width(), m_size.height());
}

void Applet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if(m_size.width() < 32)
		return; // Too small to draw a background (don't want to deal with weird corner cases).

	if(!m_interactive)
		return; // Currently, background is only used for highlight on interactive applets.

	painter->setPen(Qt::NoPen);
	qreal radius = (m_size.width()*m_size.width() + m_size.height()*m_size.height()) / (4.0*m_size.height());
	QPointF center(m_size.width()/2.0, m_size.height() + radius - m_size.height()/2.0);
	static const qreal radiusInc = 10.0;
	QRadialGradient gradient(center, radius + radiusInc, center);
	QColor highlightColor(255, 255, 255, static_cast<int>(150*m_highlightIntensity));
	gradient.setColorAt(0.0, highlightColor);
	gradient.setColorAt((radius - m_size.height()/2.0)/(radius + radiusInc), highlightColor);
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());
}

void Applet::animateHighlight()
{
	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isHighlighted() ? 1.0 : 0.0;
	bool needAnotherStep = false;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity, targetIntensity, highlightAnimationSpeed, needAnotherStep);
	if(needAnotherStep)
		QTimer::singleShot(20, this, SLOT(animateHighlight()));
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
		if(event->button() == Qt::LeftButton)
		{
			// FIXME: Workaround.
			// For some weird reason, if clicked() function is called directly, and menu is opened,
			// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
			// Probably somehow related to taking a mouse grab when one is already active.
			QTimer::singleShot(1, this, SLOT(clicked()));
		}
		if(event->button() == Qt::RightButton)
		{
			m_panelWindow->showPanelContextMenu(m_position + QPoint(static_cast<int>(event->pos().x()), static_cast<int>(event->pos().y())));
		}
	}
}
