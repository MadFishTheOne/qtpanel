#ifndef SPACERAPPLET_H
#define SPACERAPPLET_H

#include "applet.h"

class SpacerApplet: public Applet
{
	Q_OBJECT
public:
	SpacerApplet(PanelWindow* panelWindow);
	~SpacerApplet();

	QSize desiredSize();
};

#endif
