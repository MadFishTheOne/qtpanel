#include "clockapplet.h"

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelwindow.h"

ClockApplet::ClockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_timer = new QTimer();
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_textItem = new TextGraphicsItem(m_appletItem);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
}

ClockApplet::~ClockApplet()
{
	m_panelWindow->scene()->removeItem(m_textItem);
	delete m_textItem;
	delete m_timer;
}

bool ClockApplet::init()
{
	update();
	return true;
}

void ClockApplet::layoutChanged()
{
	m_textItem->setPos(8, m_panelWindow->textBaseLine());
}

void ClockApplet::update()
{
	QDateTime dateTimeNow = QDateTime::currentDateTime();
	m_text = dateTimeNow.toString();
	m_textItem->setText(m_text);
	scheduleUpdate();
	m_panelWindow->updateLayout();
}

QSize ClockApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().size().width() + 16, m_textItem->boundingRect().size().height());
}

void ClockApplet::scheduleUpdate()
{
	m_timer->setInterval(1000 - QDateTime::currentDateTime().time().msec());
	m_timer->start();
}

