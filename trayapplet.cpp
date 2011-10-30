#include "trayapplet.h"

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"

TrayItem::TrayItem(TrayApplet* trayApplet, unsigned long window)
	: m_trayApplet(trayApplet), m_window(window)
{
	setParentItem(m_trayApplet);

	// This is needed for non-composited tray icons, otherwise we'll get a BadMatch on reparent attempt.
	// Doesn't affect composited icons.
	X11Support::setWindowBackgroundBlack(m_window);

	X11Support::registerForTrayIconUpdates(m_window);
	X11Support::reparentWindow(m_window, m_trayApplet->panelWindow()->winId());
	X11Support::resizeWindow(m_window, 24, 24);
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
	X11Support::moveWindow(m_window, static_cast<int>(m_trayApplet->pos().x()) + position.x() + m_size.width()/2 - 12, static_cast<int>(m_trayApplet->pos().y()) + position.y() + m_size.height()/2 - 12);
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
	QRadialGradient gradient(center, 14.0, center);
	gradient.setColorAt(0.0, QColor(255, 255, 255, 80));
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());

	// Icon itself.
	painter->drawPixmap(m_size.width()/2 - 12, m_size.height()/2 - 12, X11Support::getWindowPixmap(m_window));
}

TrayApplet::TrayApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_initialized(false)
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

	connect(X11Support::instance(), SIGNAL(windowClosed(ulong)), this, SLOT(windowClosed(ulong)));
	connect(X11Support::instance(), SIGNAL(windowReconfigured(ulong,int,int,int,int)), this, SLOT(windowReconfigured(ulong,int,int,int,int)));
	connect(X11Support::instance(), SIGNAL(windowDamaged(ulong)), this, SLOT(windowDamaged(ulong)));
	connect(X11Support::instance(), SIGNAL(clientMessageReceived(ulong,ulong,void*)), this, SLOT(clientMessageReceived(ulong,ulong,void*)));

	return true;
}

QSize TrayApplet::desiredSize()
{
	int desiredWidth = 28*m_trayItems.size() - 4;
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

void TrayApplet::clientMessageReceived(unsigned long window, unsigned long atom, void* data)
{
	if(atom == X11Support::atom("_NET_SYSTEM_TRAY_OPCODE"))
	{
		unsigned long* l = reinterpret_cast<unsigned long*>(data);
		if(l[1] == 0) // TRAY_REQUEST_DOCK
		{
			new TrayItem(this, l[2]);
		}
	}
}

void TrayApplet::windowClosed(unsigned long window)
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

void TrayApplet::windowReconfigured(unsigned long window, int x, int y, int width, int height)
{
	for(int i = 0; i < m_trayItems.size(); i++)
	{
		if(m_trayItems[i]->window() == window)
		{
			X11Support::resizeWindow(window, 24, 24);
			break;
		}
	}
}

void TrayApplet::windowDamaged(unsigned long window)
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
		m_trayItems[i]->setSize(QSize(28, m_size.height()));
		m_trayItems[i]->setPosition(QPoint(currentPosition, 0));
		currentPosition += 28;
	}
}
