#include "dockapplet.h"

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

DockItem::DockItem(DockApplet* dockApplet)
{
	m_dockApplet = dockApplet;

	setParentItem(m_dockApplet);
	setAcceptsHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton);

	m_textItem = new TextGraphicsItem(this);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_dockApplet->panelWindow()->font());

	m_iconItem = new QGraphicsPixmapItem(this);

	m_dockApplet->registerDockItem(this);
}

DockItem::~DockItem()
{
	delete m_iconItem;
	delete m_textItem;

	m_dockApplet->unregisterDockItem(this);
}

void DockItem::updateContent()
{
	if(m_clients.isEmpty())
		return;

	QFontMetrics fontMetrics(m_textItem->font());
	QString shortName = fontMetrics.elidedText(m_clients[0]->name(), Qt::ElideRight, m_size.width() - 36);
	m_textItem->setText(shortName);
	m_textItem->setPos(28.0, m_dockApplet->panelWindow()->textBaseLine());

	m_iconItem->setPixmap(m_clients[0]->icon().pixmap(16));
	m_iconItem->setPos(8.0, m_size.height()/2 - 8);

	update();
}

void DockItem::addClient(Client* client)
{
	m_clients.append(client);
	updateContent();
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
		updateContent();
	}
}

void DockItem::setPosition(const QPoint& position)
{
	setPos(position.x(), position.y());
}

void DockItem::setSize(const QSize& size)
{
	m_size = size;
	updateContent();
}

QRectF DockItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height() - 1);
}

void DockItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	QPointF center(m_size.width()/2.0, m_size.height() + 32.0);
	QRadialGradient gradient(center, 200.0, center);
	gradient.setColorAt(0, QColor(255, 255, 255, 80 + static_cast<int>(80*m_highlightIntensity)));
	gradient.setColorAt(1, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRoundedRect(QRectF(0.0, 4.0, m_size.width(), m_size.height() - 8.0), 3.0, 3.0);
}

void DockItem::animateHighlight()
{
	static const qreal highlightAnimationSpeed = 0.15;
	if(isUnderMouse())
	{
		m_highlightIntensity += highlightAnimationSpeed;
		if(m_highlightIntensity > 1.0)
			m_highlightIntensity = 1.0;
		else
			QTimer::singleShot(20, this, SLOT(animateHighlight()));
	}
	else
	{
		m_highlightIntensity -= highlightAnimationSpeed;
		if(m_highlightIntensity < 0.0)
			m_highlightIntensity = 0.0;
		else
			QTimer::singleShot(20, this, SLOT(animateHighlight()));
	}
	update();
}

void DockItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	animateHighlight();
}

void DockItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	animateHighlight();
}

void DockItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void DockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
	{
		if(m_clients.isEmpty())
			return;

		if(m_dockApplet->activeWindow() == m_clients[0]->handle())
			X11Support::instance()->minimizeWindow(m_clients[0]->handle());
		else
			X11Support::instance()->activateWindow(m_clients[0]->handle());
	}
}

Client::Client(DockApplet* dockApplet, unsigned long handle)
	: m_dockItem(NULL)
{
	m_dockApplet = dockApplet;
	m_handle = handle;

	// Don't change input mask for dock itself.
	if(handle != m_dockApplet->panelWindow()->winId())
	{
		X11Support::instance()->registerForWindowPropertyChanges(m_handle);
	}

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
	QVector<unsigned long> windowStates = X11Support::instance()->getWindowPropertyAtomsArray(m_handle, "_NET_WM_STATE");

	// Show only regular windows in dock.
	m_visible = windowTypes.size() == 1 && windowTypes[0] == X11Support::instance()->atom("_NET_WM_WINDOW_TYPE_NORMAL");
	// Don't show window if requested explicitly in window states.
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
		m_dockItem->updateContent();
}

void Client::updateIcon()
{
	m_icon = X11Support::instance()->getWindowIcon(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->updateContent();
}

DockApplet::DockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	// Register for notifications about window property changes.
	connect(PanelApplication::instance(), SIGNAL(windowPropertyChanged(ulong,ulong)), this, SLOT(windowPropertyChanged(ulong,ulong)));
}

DockApplet::~DockApplet()
{
}

bool DockApplet::init()
{
	updateClientList();
	updateActiveWindow();

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
		m_dockItems[i]->setPosition(QPoint(currentPosition, 0));
		m_dockItems[i]->setSize(QSize(spaceForThisClient - 4, m_size.height()));
		currentPosition += spaceForThisClient;
	}

	update();
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

void DockApplet::updateClientList()
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

void DockApplet::updateActiveWindow()
{
	m_activeWindow = X11Support::instance()->getWindowPropertyWindow(X11Support::instance()->rootWindow(), "_NET_ACTIVE_WINDOW");
}

void DockApplet::windowPropertyChanged(unsigned long window, unsigned long atom)
{
	if(window == X11Support::instance()->rootWindow())
	{
		if(atom == X11Support::instance()->atom("_NET_CLIENT_LIST"))
		{
			updateClientList();
		}

		if(atom == X11Support::instance()->atom("_NET_ACTIVE_WINDOW"))
		{
			updateActiveWindow();
		}

		return;
	}

	if(!m_clients.contains(window))
		return;

	if(atom == X11Support::instance()->atom("_NET_WM_WINDOW_TYPE") || atom == X11Support::instance()->atom("_NET_WM_STATE"))
	{
		m_clients[window]->updateVisibility();
	}

	if(atom == X11Support::instance()->atom("_NET_WM_VISIBLE_NAME") || atom == X11Support::instance()->atom("_NET_WM_NAME") || atom == X11Support::instance()->atom("WM_NAME"))
	{
		m_clients[window]->updateName();
	}

	if(atom == X11Support::instance()->atom("_NET_WM_ICON"))
	{
		m_clients[window]->updateIcon();
	}
}
