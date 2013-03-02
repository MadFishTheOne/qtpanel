#include "trayapplet.h"

#include "panelwindow.h"
#include "x11support.h"
#include "dpisupport.h"

TrayApplet::TrayApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_initialized(false), m_iconSize(adjustHardcodedPixelSize(24)), m_spacing(adjustHardcodedPixelSize(4))
{
}

TrayApplet::~TrayApplet()
{
	if(m_initialized)
		X11Support::freeSystemTray();

	for(int i = 0; i < m_trayItems.size(); i++)
		X11Support::reparentWindow(m_trayItems[i], X11Support::rootWindow());
}

bool TrayApplet::init()
{
	m_initialized = X11Support::makeSystemTray(m_panelWindow->winId());

	if(!m_initialized)
	{
		// Another tray is active.
		return false;
	}

	connect(X11Support::instance(), SIGNAL(windowClosed(xcb_window_t)), this, SLOT(windowClosed(xcb_window_t)));
	connect(X11Support::instance(), SIGNAL(windowReconfigured(xcb_window_t,int,int,int,int)), this, SLOT(windowReconfigured(xcb_window_t,int,int,int,int)));
	connect(X11Support::instance(), SIGNAL(clientMessageReceived(xcb_window_t,xcb_atom_t,const uint32_t*)), this, SLOT(clientMessageReceived(xcb_window_t,xcb_atom_t,const uint32_t*)));

	return true;
}

QSize TrayApplet::desiredSize()
{
	int desiredWidth = (m_iconSize + m_spacing)*m_trayItems.size() - m_spacing;
	if(desiredWidth < 0)
		desiredWidth = 0;
	return QSize(desiredWidth, -1);
}

void TrayApplet::layoutChanged()
{
	updateLayout();
}

void TrayApplet::clientMessageReceived(xcb_window_t window, xcb_atom_t atom, const uint32_t* data)
{
	if(atom == X11Support::atom("_NET_SYSTEM_TRAY_OPCODE"))
	{
		if(data[1] == 0) // TRAY_REQUEST_DOCK
		{
			xcb_window_t window = data[2];

			if(m_trayItems.contains(window))
				return; // Already added.

			if(X11Support::registerForTrayIconUpdates(window))
			{
				X11Support::setWindowBackgroundBlack(window);

				X11Support::reparentWindow(window, m_panelWindow->winId());
				X11Support::resizeWindow(window, m_iconSize, m_iconSize);
				X11Support::mapWindow(window);

				m_trayItems.append(window);
				m_panelWindow->updateLayout();
			}
		}
	}
}

void TrayApplet::windowClosed(xcb_window_t window)
{
	if(m_trayItems.contains(window))
	{
		m_trayItems.remove(m_trayItems.indexOf(window));
		m_panelWindow->updateLayout();
	}
}

void TrayApplet::windowReconfigured(xcb_window_t window, int x, int y, int width, int height)
{
	if(m_trayItems.contains(window))
		X11Support::resizeWindow(window, m_iconSize, m_iconSize);
}

void TrayApplet::updateLayout()
{
	int currentPosition = 0;
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		X11Support::moveWindow(m_trayItems[i], m_position.x() + currentPosition, m_position.y() + m_size.height()/2 - m_iconSize/2);
		currentPosition += m_iconSize + m_spacing;
	}
}
