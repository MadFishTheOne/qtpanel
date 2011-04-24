#include "trayapplet.h"

#include <QtGui/QX11EmbedContainer>
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

// FIXME: Tray applet isn't usable yet. Needs more investigation.
// Won't work when panel window background is set to transparent (probably visual mismatch).
// Also, there are problems with QX11EmbedContainer. For some reason, tray icon window's minimal size hint
// was set to 48x48 in my test case and that's why QX11EmbedContainer didn't resize it correctly (to smaller size).

TrayItem::TrayItem(TrayApplet* trayApplet, unsigned long window)
	: m_trayApplet(trayApplet), m_window(window)
{
	m_container = new QX11EmbedContainer(trayApplet->panelWindow());

	connect(m_container, SIGNAL(clientClosed()), this, SLOT(clientClosed()));
	connect(m_container, SIGNAL(clientIsEmbedded()), this, SLOT(clientIsEmbedded()));

	m_container->show();
	m_container->embedClient(m_window);

	m_trayApplet->registerTrayItem(this);
}

TrayItem::~TrayItem()
{
	delete m_container;

	m_trayApplet->unregisterTrayItem(this);
}

void TrayItem::setPosition(const QPoint& position)
{
	m_container->move(static_cast<int>(m_trayApplet->pos().x() + position.x()), static_cast<int>(m_trayApplet->pos().y() + position.y()));
}

void TrayItem::setSize(const QSize& size)
{
	m_size = size;
	if(m_container->clientWinId() != 0)
		m_container->resize(m_size);
}

void TrayItem::clientClosed()
{
	delete this;
}

void TrayItem::clientIsEmbedded()
{
	m_container->resize(m_size);
}

TrayApplet::TrayApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_initialized(false)
{
}

TrayApplet::~TrayApplet()
{
	if(m_initialized)
		X11Support::instance()->freeSystemTray();

	while(!m_trayItems.isEmpty())
	{
		delete m_trayItems[m_trayItems.size() - 1];
	}
}

bool TrayApplet::init()
{
	m_initialized = X11Support::instance()->makeSystemTray(m_panelWindow->winId());

	if(!m_initialized)
	{
		// Another tray is active.
		return false;
	}

	connect(PanelApplication::instance(), SIGNAL(clientMessageReceived(ulong,ulong,void*)), this, SLOT(clientMessageReceived(ulong,ulong,void*)));

	return true;
}

QSize TrayApplet::desiredSize()
{
	return QSize(32*m_trayItems.size(), -1);
}

void TrayApplet::registerTrayItem(TrayItem* trayItem)
{
	m_trayItems.append(trayItem);
	m_panelWindow->updateLayout();
}

void TrayApplet::unregisterTrayItem(TrayItem* trayItem)
{
	m_trayItems.remove(m_trayItems.indexOf(trayItem));
	m_panelWindow->updateLayout();
}

void TrayApplet::layoutChanged()
{
	updateLayout();
}

void TrayApplet::clientMessageReceived(unsigned long window, unsigned long atom, void* data)
{
	if(atom == X11Support::instance()->atom("_NET_SYSTEM_TRAY_OPCODE"))
	{
		unsigned long* l = reinterpret_cast<unsigned long*>(data);
		if(l[1] == 0) // TRAY_REQUEST_DOCK
		{
			new TrayItem(this, l[2]);
		}
	}
}

void TrayApplet::updateLayout()
{
	int currentPosition = 0;
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		m_trayItems[i]->setPosition(QPoint(currentPosition, m_size.height()/2 - 12));
		m_trayItems[i]->setSize(QSize(24, 24));
		currentPosition += 24;
	}
}
