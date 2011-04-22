#include "dockapplet.h"

#include <QtGui/QGraphicsScene>
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

#include <stdio.h>

Client::Client(DockApplet* dockApplet, unsigned long handle)
{
	m_dockApplet = dockApplet;
	m_dockApplet->registerClient(this);
	m_handle = handle;
	m_name = X11Support::instance()->getWindowName(m_handle);

	fprintf(stderr, "New client: %s\n", m_name.toUtf8().data());
}

Client::~Client()
{
	m_dockApplet->unregisterClient(this);
}

void Client::removed()
{
	fprintf(stderr, "Client removed: %s\n", m_name.toUtf8().data());

	// Call destructor for now.
	// TODO: Animations.
	delete this;
}

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

void DockApplet::registerClient(Client* client)
{
	m_clients.append(client);
}

void DockApplet::unregisterClient(Client* client)
{
	m_clients.remove(m_clients.indexOf(client));
}

void DockApplet::clientListChanged()
{
	QVector<unsigned long> windows = X11Support::instance()->getWindowPropertyWindowsArray(X11Support::instance()->rootWindow(), "_NET_CLIENT_LIST");

	// Handle new clients.
	for(int i = 0; i < windows.size(); i++)
	{
		bool found = false;
		for(int k = 0; k < m_clients.size(); k++)
		{
			if(m_clients[k]->handle() == windows[i])
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			Client* client = new Client(this, windows[i]);
		}
	}

	// Handle removed clients.
	for(int i = 0; i < m_clients.size(); i++)
	{
		bool found = false;
		for(int k = 0; k < windows.size(); k++)
		{
			if(m_clients[i]->handle() == windows[k])
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			m_clients[i]->removed();
		}
	}
}
