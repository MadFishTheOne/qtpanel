#include "dockapplet.h"

#include <QtGui/QGraphicsScene>
#include "panelwindow.h"
#include "x11support.h"

#include <stdio.h>

DockApplet::DockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
}

DockApplet::~DockApplet()
{
}

bool DockApplet::init()
{
	QVector<unsigned long> windows = X11Support::instance()->getWindowPropertyWindowsArray(X11Support::instance()->rootWindow(), "_NET_CLIENT_LIST");
	for(int i = 0; i < windows.size(); i++)
	{
		fprintf(stderr, "%s\n", X11Support::instance()->getWindowName(windows[i]).toUtf8().data());
	}
	return true;
}

void DockApplet::layoutChanged()
{
}

QSize DockApplet::desiredSize()
{
	return QSize(-1, -1); // Take all available space.
}
