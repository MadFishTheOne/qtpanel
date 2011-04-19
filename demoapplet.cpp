#include "demoapplet.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include "panelwindow.h"

DemoApplet::DemoApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_rectItem = new QGraphicsRectItem();
	m_rectItem->setBrush(QBrush(Qt::blue));
	m_panelWindow->scene()->addItem(m_rectItem);
}

DemoApplet::~DemoApplet()
{
	m_panelWindow->scene()->removeItem(m_rectItem);
	delete m_rectItem;
}

void DemoApplet::update()
{
	static const int delta = 8;
	m_rectItem->setRect(m_rect.left() + delta, m_rect.top() + delta, m_rect.width() - 2*delta, m_rect.height() - 2*delta);
}

QSize DemoApplet::desiredSize()
{
	return QSize(64, 64);
}
