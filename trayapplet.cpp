#include "trayapplet.h"

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"
#include "dpisupport.h"

TrayItem::TrayItem(TrayApplet* trayApplet, xcb_window_t window)
	: m_trayApplet(trayApplet), m_window(window)
{
	setParentItem(m_trayApplet);

	// This is needed for non-composited tray icons, otherwise we'll get a BadMatch on reparent attempt.
	// Doesn't affect composited icons.
	X11Support::setWindowBackgroundBlack(m_window);

	X11Support::registerForTrayIconUpdates(m_window);
	X11Support::reparentWindow(m_window, m_trayApplet->panelWindow()->winId());
	X11Support::resizeWindow(m_window, m_trayApplet->iconSize(), m_trayApplet->iconSize());
	X11Support::redirectWindow(m_window);
	X11Support::mapWindow(m_window);

	m_trayApplet->registerTrayItem(this);
}

TrayItem::~TrayItem()
{
	X11Support::reparentWindow(m_window, X11Support::rootWindow());

	m_trayApplet->unregisterTrayItem(this);
}

void TrayItem::setPosition(const QPoint& position)
{
	setPos(position.x(), position.y());
	X11Support::moveWindow(m_window,
		static_cast<int>(m_trayApplet->pos().x()) + position.x() + m_size.width()/2 - m_trayApplet->iconSize()/2,
		static_cast<int>(m_trayApplet->pos().y()) + position.y() + m_size.height()/2 - m_trayApplet->iconSize()/2
	);
}

void TrayItem::setSize(const QSize& size)
{
	m_size = size;
	update();
}

QRectF TrayItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height() - 1);
}

void TrayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// Background.
	painter->setPen(Qt::NoPen);
	QPointF center(m_size.width()/2.0, m_size.height()/2.0);
	QRadialGradient gradient(center, m_size.width()/2.0, center);
	gradient.setColorAt(0.0, QColor(255, 255, 255, 80));
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());

	// Icon itself.
	painter->drawPixmap(m_size.width()/2 - m_trayApplet->iconSize()/2, m_size.height()/2 - m_trayApplet->iconSize()/2, X11Support::getWindowPixmap(m_window));
}

TrayApplet::TrayApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_initialized(false), m_iconSize(adjustHardcodedPixelSize(24)), m_spacing(adjustHardcodedPixelSize(4))
{
}

TrayApplet::~TrayApplet()
{
	if(m_initialized)
		X11Support::freeSystemTray();

	while(!m_trayItems.isEmpty())
	{
		delete m_trayItems[m_trayItems.size() - 1];
	}
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
	connect(X11Support::instance(), SIGNAL(windowDamaged(xcb_window_t)), this, SLOT(windowDamaged(xcb_window_t)));
	connect(X11Support::instance(), SIGNAL(clientMessageReceived(xcb_window_t,xcb_atom_t,void*)), this, SLOT(clientMessageReceived(xcb_window_t,xcb_atom_t,void*)));

	return true;
}

QSize TrayApplet::desiredSize()
{
	int desiredWidth = (m_iconSize + m_spacing)*m_trayItems.size() - m_spacing;
	if(desiredWidth < 0)
		desiredWidth = 0;
	return QSize(desiredWidth, -1);
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

void TrayApplet::clientMessageReceived(xcb_window_t window, xcb_atom_t atom, void* data)
{
	if(atom == X11Support::atom("_NET_SYSTEM_TRAY_OPCODE"))
	{
		uint32_t* l = reinterpret_cast<uint32_t*>(data);
		if(l[1] == 0) // TRAY_REQUEST_DOCK
		{
			for(int i = 0; i < m_trayItems.size(); i++)
			{
				if(m_trayItems[i]->window() == l[2])
					return; // Already added.
			}
			new TrayItem(this, l[2]);
		}
	}
}

void TrayApplet::windowClosed(xcb_window_t window)
{
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		if(m_trayItems[i]->window() == window)
		{
			delete m_trayItems[i];
			break;
		}
	}
}

void TrayApplet::windowReconfigured(xcb_window_t window, int x, int y, int width, int height)
{
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		if(m_trayItems[i]->window() == window)
		{
			X11Support::resizeWindow(window, m_iconSize, m_iconSize);
			break;
		}
	}
}

void TrayApplet::windowDamaged(xcb_window_t window)
{
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		if(m_trayItems[i]->window() == window)
		{
			m_trayItems[i]->update();
			break;
		}
	}
}

void TrayApplet::updateLayout()
{
	int currentPosition = 0;
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		m_trayItems[i]->setSize(QSize(m_iconSize, m_size.height()));
		m_trayItems[i]->setPosition(QPoint(currentPosition, 0));
		currentPosition += m_iconSize + m_spacing;
	}
}
