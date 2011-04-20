#include "clockapplet.h"

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsTextItem>
#include "panelwindow.h"

ClockApplet::ClockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_timer = new QTimer();
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_textItem = new QGraphicsTextItem();
	m_textItem->setDefaultTextColor(Qt::white);
	m_panelWindow->scene()->addItem(m_textItem);
}

ClockApplet::~ClockApplet()
{
	m_panelWindow->scene()->removeItem(m_textItem);
	delete m_textItem;
	delete m_timer;
}

bool ClockApplet::init()
{
	scheduleUpdate();
	return true;
}

void ClockApplet::update()
{
	m_textItem->setPos(m_rect.topLeft());
	QDateTime dateTimeNow = QDateTime::currentDateTime();
	m_textItem->setPlainText(dateTimeNow.toString());
	scheduleUpdate();
}

QSize ClockApplet::desiredSize()
{
	return QSize(192, -1);
}

void ClockApplet::scheduleUpdate()
{
	m_timer->setInterval(1000 - QDateTime::currentDateTime().time().msec());
	m_timer->start();
}

