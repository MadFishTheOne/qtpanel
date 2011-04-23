#include "dockapplet.h"

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

DockItemGraphicsItem::DockItemGraphicsItem(DockItem* dockItem)
	: m_dockItem(dockItem)
{
}

DockItemGraphicsItem::~DockItemGraphicsItem()
{
}

QRectF DockItemGraphicsItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_dockItem->size().width() - 1, m_dockItem->size().height() - 1);
}

void DockItemGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	QPointF center(m_dockItem->size().width()/2.0, m_dockItem->size().height() + 32.0);
	QRadialGradient gradient(center, 200.0, center);
	gradient.setColorAt(0, QColor(255, 255, 255, 80));
	gradient.setColorAt(1, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRoundedRect(boundingRect(), 3.0, 3.0);
}

DockItem::DockItem(DockApplet* dockApplet)
{
	m_dockApplet = dockApplet;

	m_graphicsItem = new DockItemGraphicsItem(this);
	m_graphicsItem->setParentItem(m_dockApplet->appletItem());

	m_textItem = new TextGraphicsItem(m_graphicsItem);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_dockApplet->panelWindow()->font());

	m_iconItem = new QGraphicsPixmapItem(m_graphicsItem);

	m_dockApplet->registerDockItem(this);
}

DockItem::~DockItem()
{
	delete m_iconItem;
	delete m_textItem;
	delete m_graphicsItem;

	m_dockApplet->unregisterDockItem(this);
}

void DockItem::update()
{
	if(m_clients.isEmpty())
		return;

	QFontMetrics fontMetrics(m_textItem->font());
	QString shortName = fontMetrics.elidedText(m_clients[0]->name(), Qt::ElideRight, m_size.width() - 36);
	m_textItem->setText(shortName);
	m_textItem->setPos(28.0, m_dockApplet->panelWindow()->textBaseLine() - 4.0);

	m_iconItem->setPixmap(m_clients[0]->icon().pixmap(16));
	m_iconItem->setPos(8.0, m_size.height()/2 - 8);
}

void DockItem::addClient(Client* client)
{
	m_clients.append(client);
	update();
}

void DockItem::removeClient(Client* client)
{
	m_clients.remove(m_clients.indexOf(client));
	if(m_clients.isEmpty())
	{
		// TODO: Stub. Item may be a launcher.
		delete this;
	}
	else
	{
		update();
	}
}

void DockItem::setPosition(const QPoint& position)
{
	m_graphicsItem->setPos(position.x(), position.y());
}

void DockItem::setSize(const QSize& size)
{
	m_size = size;
	update();
}

Client::Client(DockApplet* dockApplet, unsigned long handle)
	: m_dockItem(NULL)
{
	m_dockApplet = dockApplet;
	m_handle = handle;

	updateVisibility();
	updateName();
	updateIcon();
}

Client::~Client()
{
	if(m_dockItem != NULL)
	{
		m_dockItem->removeClient(this);
	}
}

void Client::updateVisibility()
{
	QVector<unsigned long> windowTypes = X11Support::instance()->getWindowPropertyAtomsArray(m_handle, "_NET_WM_WINDOW_TYPE");
	m_visible = windowTypes.contains(X11Support::instance()->atom("_NET_WM_WINDOW_TYPE_NORMAL"));
	QVector<unsigned long> windowStates = X11Support::instance()->getWindowPropertyAtomsArray(m_handle, "_NET_WM_STATE");
	if(windowStates.contains(X11Support::instance()->atom("_NET_WM_STATE_SKIP_TASKBAR")))
		m_visible = false;

	if(m_dockItem == NULL && m_visible)
	{
		m_dockItem = m_dockApplet->dockItemForClient(this);
		m_dockItem->addClient(this);
	}

	if(m_dockItem != NULL && !m_visible)
	{
		m_dockItem->removeClient(this);
		m_dockItem = NULL;
	}
}

void Client::updateName()
{
	m_name = X11Support::instance()->getWindowName(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->update();
}

void Client::updateIcon()
{
	m_icon = X11Support::instance()->getWindowIcon(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->update();
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

	int freeSpace = m_size.width() - 8;
	int spaceForOneClient = (m_dockItems.size() > 0) ? freeSpace/m_dockItems.size() : 0;
	int currentPosition = 4;
	for(int i = 0; i < m_dockItems.size(); i++)
	{
		int spaceForThisClient = spaceForOneClient;
		if(spaceForThisClient > 256)
			spaceForThisClient = 256;
		m_dockItems[i]->setPosition(QPoint(currentPosition, 4));
		m_dockItems[i]->setSize(QSize(spaceForThisClient - 4, m_size.height() - 8));
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

void DockApplet::registerDockItem(DockItem* dockItem)
{
	m_dockItems.append(dockItem);
	updateLayout();
}

void DockApplet::unregisterDockItem(DockItem* dockItem)
{
	m_dockItems.remove(m_dockItems.indexOf(dockItem));
	updateLayout();
}

DockItem* DockApplet::dockItemForClient(Client* client)
{
	// FIXME: Stub.
	return new DockItem(this);
}

void DockApplet::clientListChanged()
{
	QVector<unsigned long> windows = X11Support::instance()->getWindowPropertyWindowsArray(X11Support::instance()->rootWindow(), "_NET_CLIENT_LIST");

	// Handle new clients.
	for(int i = 0; i < windows.size(); i++)
	{
		if(!m_clients.contains(windows[i]))
		{
			m_clients[windows[i]] = new Client(this, windows[i]);
		}
	}

	// Handle removed clients.
	for(;;)
	{
		bool clientRemoved = false;
		foreach(Client* client, m_clients)
		{
			int handle = client->handle();
			if(!windows.contains(handle))
			{
				delete m_clients[handle];
				m_clients.remove(handle);
				clientRemoved = true;
				break;
			}
		}
		if(!clientRemoved)
			break;
	}
}
