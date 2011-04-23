#include "dockapplet.h"

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

ClientGraphicsItem::ClientGraphicsItem(Client* client)
	: m_client(client)
{

}

ClientGraphicsItem::~ClientGraphicsItem()
{

}

QRectF ClientGraphicsItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_client->size().width() - 1, m_client->size().height() - 1);
}

void ClientGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	QPointF center(m_client->size().width()/2.0, m_client->size().height() + 32.0);
	QRadialGradient gradient(center, 200.0, center);
	gradient.setColorAt(0, QColor(255, 255, 255, 80));
	gradient.setColorAt(1, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRoundedRect(boundingRect(), 3.0, 3.0);
}

Client::Client(DockApplet* dockApplet, unsigned long handle)
{
	m_dockApplet = dockApplet;
	m_handle = handle;

	m_clientItem = new ClientGraphicsItem(this);

	m_textItem = new TextGraphicsItem(m_clientItem);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_dockApplet->panelWindow()->font());

	m_iconItem = new QGraphicsPixmapItem(m_clientItem);

	updateName();
	updateVisibility();
	updateIcon();

	m_dockApplet->registerClient(this);
	m_dockApplet->updateLayout();
}

Client::~Client()
{
	delete m_iconItem;
	delete m_textItem;
	delete m_clientItem;

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
	m_clientItem->setPos(position.x(), position.y());
}

void Client::setSize(const QSize& size)
{
	m_size = size;
	updateLayout();
}

void Client::updateLayout()
{
	updateTextItem();

	m_textItem->setPos(28.0, m_dockApplet->panelWindow()->textBaseLine() - 4.0);
	m_iconItem->setPos(8.0, m_size.height()/2 - 8);
}

void Client::updateName()
{
	m_name = X11Support::instance()->getWindowName(m_handle);
	updateTextItem();
}

void Client::updateVisibility()
{
	QVector<unsigned long> windowTypes = X11Support::instance()->getWindowPropertyAtomsArray(m_handle, "_NET_WM_WINDOW_TYPE");
	m_visible = windowTypes.contains(X11Support::instance()->atom("_NET_WM_WINDOW_TYPE_NORMAL"));
	QVector<unsigned long> windowStates = X11Support::instance()->getWindowPropertyAtomsArray(m_handle, "_NET_WM_STATE");
	if(windowStates.contains(X11Support::instance()->atom("_NET_WM_STATE_SKIP_TASKBAR")))
		m_visible = false;

	if(m_visible)
		m_clientItem->setParentItem(m_dockApplet->appletItem());
	else
		m_clientItem->setParentItem(NULL);
}

void Client::updateIcon()
{
	m_icon = X11Support::instance()->getWindowIcon(m_handle);
	m_iconItem->setPixmap(m_icon.pixmap(16));
}

void Client::updateTextItem()
{
	QFontMetrics fontMetrics(m_textItem->font());
	QString shortName = fontMetrics.elidedText(m_name, Qt::ElideRight, m_size.width() - 36);
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

	int numVisibleClients = 0;
	for(int i = 0; i < m_clients.size(); i++)
	{
		if(m_clients[i]->isVisible())
			numVisibleClients++;
	}

	int freeSpace = m_size.width() - 8;
	int spaceForOneClient = (numVisibleClients > 0) ? freeSpace/numVisibleClients : 0;
	int currentPosition = 4;
	for(int i = 0; i < m_clients.size(); i++)
	{
		if(!m_clients[i]->isVisible())
			continue;

		int spaceForThisClient = spaceForOneClient;
		if(m_clients[i]->desiredSize().width() < spaceForThisClient)
			spaceForThisClient = m_clients[i]->desiredSize().width();
		m_clients[i]->setPosition(QPoint(currentPosition, 4));
		m_clients[i]->setSize(QSize(spaceForThisClient - 4, m_size.height() - 8));
		currentPosition += spaceForThisClient;
	}

	m_appletItem->update();
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
