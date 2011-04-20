#ifndef CLOCKAPPLET_H
#define CLOCKAPPLET_H

#include "applet.h"

class QTimer;
class QGraphicsTextItem;

class ClockApplet: public Applet
{
	Q_OBJECT
public:
	ClockApplet(PanelWindow* panelWindow);
	~ClockApplet();

	bool init();
	void update();
	QSize desiredSize();

private:
	void scheduleUpdate();

	QTimer* m_timer;
	QGraphicsTextItem* m_textItem;
};

#endif
