#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include "applet.h"

class TextGraphicsItem;

class ApplicationsMenuApplet: public Applet
{
	Q_OBJECT
public:
	ApplicationsMenuApplet(PanelWindow* panelWindow);
	~ApplicationsMenuApplet();

	bool init();
	QSize desiredSize();

protected:
	void layoutChanged();

	TextGraphicsItem* m_textItem;
};

#endif
