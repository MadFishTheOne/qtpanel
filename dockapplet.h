#ifndef DOCKAPPLET_H
#define DOCKAPPLET_H

#include "applet.h"

class DockApplet: public Applet
{
	Q_OBJECT
public:
	DockApplet(PanelWindow* panelWindow);
	~DockApplet();

	bool init();
	QSize desiredSize();

protected:
	void layoutChanged();

private:
};

#endif
