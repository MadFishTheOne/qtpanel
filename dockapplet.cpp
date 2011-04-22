#include "dockapplet.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

Client::Client(DockApplet* dockApplet, unsigned long handle)
{
	m_dockApplet = dockApplet;
	m_handle = handle;
	m_name = X11Support::instance()->getWindowName(m_handle);

	m_textItem = new TextGraphicsItem(m_dockApplet->appletItem());
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_dockApplet->panelWindow()->font());

	m_dockApplet->registerClient(this);
	m_dockApplet->updateLayout();
}

Client::~Client()
{
	delete m_textItem;

	m_dockApplet->unregisterClient(this);
	m_dockApplet->updateLayout();
}

void Client::removed()
{
	// Call destructor for now.
	// TODO: Animations.
	delete this;
}

QSize Client::desiredSize()
{
	return QSize(256, 256); // TODO: Make it configurable;
}

void Client::setPosition(const QPoint& position)
{
	m_textItem->setPos(position.x(), position.y() + m_dockApplet->panelWindow()->textBaseLine());
}

void Client::setSize(const QSize& size)
{
	m_size = size;
	updateLayout();
}

void Client::updateLayout()
{
	QFontMetrics fontMetrics(m_textItem->font());
	QString shortName = fontMetrics.elidedText(m_name, Qt::ElideRight, m_size.width());
	m_textItem->setText(shortName);
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

void DockApplet::updateLayout()
{
	// TODO: Vertical orientation support.
	int freeSpace = m_size.width();
	int spaceForOneClient = (m_clients.size() > 0) ? freeSpace/m_clients.size() : 0;
	int currentPosition = 0;
	for(int i = 0; i < m_clients.size(); i++)
	{
		int spaceForThisClient = spaceForOneClient;
		if(m_clients[i]->desiredSize().width() < spaceForThisClient)
			spaceForThisClient = m_clients[i]->desiredSize().width();
		m_clients[i]->setPosition(QPoint(currentPosition, 0));
		m_clients[i]->setSize(QSize(spaceForThisClient, m_size.height()));
		currentPosition += spaceForThisClient;
	}
}

void DockApplet::layoutChanged()
{
	updateLayout();
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
