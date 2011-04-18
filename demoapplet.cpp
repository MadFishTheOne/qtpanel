#include "demoapplet.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include "panelwindow.h"

DemoApplet::DemoApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	rect = new QGraphicsRectItem();
	rect->setBrush(QBrush(Qt::blue));
	m_panelWindow->scene()->addItem(rect);
}

DemoApplet::~DemoApplet()
{
	m_panelWindow->scene()->removeItem(rect);
	delete rect;
}

bool DemoApplet::init()
{
	emit desiredSizeChanged(QSize(128, 128));
	return true;
}

void DemoApplet::update()
{
	static const int delta = 8;
	rect->setRect(delta, delta, m_size.width() - delta, m_size.height() - delta);
}
