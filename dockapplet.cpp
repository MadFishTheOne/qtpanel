#include "dockapplet.h"

#include <QtGui/QGraphicsScene>
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

#include <stdio.h>

DockApplet::DockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	// Register for notifications about client list changes.
	connect(PanelApplication::instance(), SIGNAL(clientListChanged()), this, SLOT(clientListChanged()));
}

DockApplet::~DockApplet()
{
}

bool DockApplet::init()
{
	// Get info about existing clients.
	clientListChanged();

	return true;
}

void DockApplet::layoutChanged()
{
}

QSize DockApplet::desiredSize()
{
	return QSize(-1, -1); // Take all available space.
}

void DockApplet::clientListChanged()
{
	fprintf(stderr, "New client list:\n");
	QVector<unsigned long> windows = X11Support::instance()->getWindowPropertyWindowsArray(X11Support::instance()->rootWindow(), "_NET_CLIENT_LIST");
	for(int i = 0; i < windows.size(); i++)
	{
		fprintf(stderr, "%s\n", X11Support::instance()->getWindowName(windows[i]).toUtf8().data());
	}
}
