#include "demoapplet.h"

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include "panelwindow.h"

DemoApplet::DemoApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_rectItem = new QGraphicsRectItem(this);
	m_rectItem->setPen(QPen(Qt::NoPen));
	m_rectItem->setBrush(QBrush(Qt::blue));
}

DemoApplet::~DemoApplet()
{
	delete m_rectItem;
}

void DemoApplet::layoutChanged()
{
	static const int delta = 8;
	m_rectItem->setRect(delta, delta, m_size.width() - 2*delta, m_size.height() - 2*delta);
}

QSize DemoApplet::desiredSize()
{
	return QSize(64, 64);
}
