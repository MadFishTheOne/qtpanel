#ifndef DEMOAPPLET_H
#define DEMOAPPLET_H

#include "applet.h"

class QGraphicsRectItem;

class DemoApplet: public Applet
{
	Q_OBJECT
public:
	DemoApplet(PanelWindow* panelWindow);
	~DemoApplet();

	void update();
	QSize desiredSize();

private:
	QGraphicsRectItem* m_rectItem;
};

#endif
