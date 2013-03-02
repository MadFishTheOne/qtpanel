#include "dockapplet.h"

#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QMenu>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
#include "x11support.h"
#include "animationutils.h"
#include "dpisupport.h"

DockItem::DockItem(DockApplet* dockApplet)
	: m_dragging(false), m_highlightIntensity(0.0), m_urgencyHighlightIntensity(0.0)
{
	m_dockApplet = dockApplet;

	m_animationTimer = new QTimer();

	m_animationTimer->setInterval(20);
	m_animationTimer->setSingleShot(true);
	connect(m_animationTimer, SIGNAL(timeout()), this, SLOT(animate()));

	setParentItem(m_dockApplet);
	setAcceptHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

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
	delete m_animationTimer;

	m_dockApplet->unregisterDockItem(this);
}

void DockItem::updateContent()
{
	if(m_clients.isEmpty())
		return;

	QFontMetrics fontMetrics(m_textItem->font());
	QString shortName = fontMetrics.elidedText(m_clients[0]->name(), Qt::ElideRight, m_targetSize.width() - adjustHardcodedPixelSize(36));
	m_textItem->setText(shortName);
	m_textItem->setPos(adjustHardcodedPixelSize(28), m_dockApplet->panelWindow()->textBaseLine());

	m_iconItem->setPixmap(m_clients[0]->icon().pixmap(adjustHardcodedPixelSize(16)));
	m_iconItem->setPos(adjustHardcodedPixelSize(8), m_targetSize.height()/2 - adjustHardcodedPixelSize(8));

	update();
}

void DockItem::addClient(Client* client)
{
	m_clients.append(client);
	updateClientsIconGeometry();
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

void DockItem::setTargetPosition(const QPoint& targetPosition)
{
	m_targetPosition = targetPosition;
	updateClientsIconGeometry();
}

void DockItem::setTargetSize(const QSize& targetSize)
{
	m_targetSize = targetSize;
	updateClientsIconGeometry();
	updateContent();
}

void DockItem::moveInstantly()
{
	m_position = m_targetPosition;
	m_size = m_targetSize;
	setPos(m_position.x(), m_position.y());
	update();
}

void DockItem::startAnimation()
{
	if(!m_animationTimer->isActive())
		m_animationTimer->start();
}

void DockItem::animate()
{
	bool needAnotherStep = false;

	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isUnderMouse() ? 1.0 : 0.0;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity, targetIntensity, highlightAnimationSpeed, needAnotherStep);

	static const qreal urgencyHighlightAnimationSpeed = 0.015;
	qreal targetUrgencyIntensity = 0.0;
	if(isUrgent())
	{
		qint64 msecs = QDateTime::currentMSecsSinceEpoch() % 3000;
		if(msecs < 1500)
			targetUrgencyIntensity = 1.0;
		else
			targetUrgencyIntensity = 0.5;
		needAnotherStep = true;
	}
	m_urgencyHighlightIntensity = AnimationUtils::animate(m_urgencyHighlightIntensity, targetUrgencyIntensity, urgencyHighlightAnimationSpeed, needAnotherStep);

	if(!m_dragging)
	{
		static const int positionAnimationSpeed = 24;
		static const int sizeAnimationSpeed = 24;
		m_position.setX(AnimationUtils::animateExponentially(m_position.x(), m_targetPosition.x(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_position.setY(AnimationUtils::animateExponentially(m_position.y(), m_targetPosition.y(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_size.setWidth(AnimationUtils::animate(m_size.width(), m_targetSize.width(), sizeAnimationSpeed, needAnotherStep));
		m_size.setHeight(AnimationUtils::animate(m_size.height(), m_targetSize.height(), sizeAnimationSpeed, needAnotherStep));
		setPos(m_position.x(), m_position.y());
	}

	update();

	if(needAnotherStep)
		m_animationTimer->start();
}

void DockItem::close()
{
	for(int i = 0; i < m_clients.size(); i++)
	{
		X11Support::closeWindow(m_clients[i]->handle());
	}
}

QRectF DockItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height() - 1);
}

void DockItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	QPointF center(m_size.width()/2.0, m_size.height() + adjustHardcodedPixelSize(32));
	QRectF rect(0.0, adjustHardcodedPixelSize(4), m_size.width(), m_size.height() - adjustHardcodedPixelSize(8));
	static const qreal roundRadius = adjustHardcodedPixelSize(3);

	{
		QRadialGradient gradient(center, adjustHardcodedPixelSize(200), center);
		gradient.setColorAt(0.0, QColor(255, 255, 255, 80 + static_cast<int>(80*m_highlightIntensity)));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRoundedRect(rect, roundRadius, roundRadius);
	}

	if(m_urgencyHighlightIntensity > 0.001)
	{
		QRadialGradient gradient(center, adjustHardcodedPixelSize(200), center);
		gradient.setColorAt(0.0, QColor(255, 100, 0, static_cast<int>(160*m_urgencyHighlightIntensity)));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRoundedRect(rect, roundRadius, roundRadius);
	}
}

void DockItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	startAnimation();
}

void DockItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	startAnimation();
}

void DockItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		m_dragging = true;
		m_mouseDownPosition = event->scenePos();
		m_dragStartPosition = m_position;
		m_dockApplet->draggingStarted();
		setZValue(1.0); // Be on top when dragging.
	}
}

void DockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		m_dragging = false;
		m_dockApplet->draggingStopped();
		setZValue(0.0); // No more on top.
		startAnimation(); // Item can be out of it's regular, start animation to bring it back.
	}

	if(isUnderMouse())
	{
		if(m_clients.isEmpty())
			return;

		if(event->button() == Qt::LeftButton)
		{
			static const qreal clickMouseMoveTolerance = 10.0;

			if((event->scenePos() - m_mouseDownPosition).manhattanLength() < clickMouseMoveTolerance)
			{
				if(m_dockApplet->activeWindow() == m_clients[0]->handle())
					X11Support::minimizeWindow(m_clients[0]->handle());
				else
					X11Support::activateWindow(m_clients[0]->handle());
			}
		}

		if(event->button() == Qt::RightButton && !m_dragging)
		{
			QMenu menu;
			menu.addAction(QIcon::fromTheme("window-close"), "Close", this, SLOT(close()));
			menu.exec(event->screenPos());
		}
	}
}

void DockItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	// Mouse events are sent only when mouse button is pressed.

	if(!m_dragging)
		return;

	// TODO: Vertical orientation support.

	QPointF delta = event->scenePos() - m_mouseDownPosition;
	m_position.setX(m_dragStartPosition.x() + static_cast<int>(delta.x()));
	if(m_position.x() < 0)
		m_position.setX(0);
	if(m_position.x() >= m_dockApplet->size().width() - m_targetSize.width())
		m_position.setX(m_dockApplet->size().width() - m_targetSize.width());
	setPos(m_position.x(), m_position.y());

	int criticalShift = m_targetSize.width()*55/100;

	if(m_position.x() < m_targetPosition.x() - criticalShift)
		m_dockApplet->moveItem(this, false);

	if(m_position.x() > m_targetPosition.x() + criticalShift)
		m_dockApplet->moveItem(this, true);

	update();
}

void DockItem::updateClientsIconGeometry()
{
	QPointF topLeft = m_dockApplet->mapToScene(m_targetPosition);
	QVector<uint32_t> values;
	values.resize(4);
	values[0] = static_cast<uint32_t>(topLeft.x()) + m_dockApplet->panelWindow()->pos().x();
	values[1] = static_cast<uint32_t>(topLeft.y()) + m_dockApplet->panelWindow()->pos().y();
	values[2] = m_targetSize.width();
	values[3] = m_targetSize.height();
	for(int i = 0; i < m_clients.size(); i++)
	{
		X11Support::setWindowPropertyCardinalArray(m_clients[i]->handle(), "_NET_WM_ICON_GEOMETRY", values);
	}
}

bool DockItem::isUrgent()
{
	for(int i = 0; i < m_clients.size(); i++)
	{
		if(m_clients[i]->isUrgent())
			return true;
	}
	return false;
}

Client::Client(DockApplet* dockApplet, xcb_window_t handle)
	: m_dockItem(NULL)
{
	m_dockApplet = dockApplet;
	m_handle = handle;

	X11Support::registerForWindowPropertyChanges(m_handle);

	updateVisibility();
	updateName();
	updateIcon();
	updateUrgency();
}

Client::~Client()
{
	if(m_dockItem != NULL)
	{
		m_dockItem->removeClient(this);
	}
}

void Client::windowPropertyChanged(xcb_atom_t atom)
{
	if(atom == X11Support::atom("_NET_WM_WINDOW_TYPE") || atom == X11Support::atom("_NET_WM_STATE"))
	{
		updateVisibility();
	}

	if(atom == X11Support::atom("_NET_WM_VISIBLE_NAME") || atom == X11Support::atom("_NET_WM_NAME") || atom == X11Support::atom("WM_NAME"))
	{
		updateName();
	}

	if(atom == X11Support::atom("_NET_WM_ICON"))
	{
		updateIcon();
	}

	if(atom == X11Support::atom("WM_HINTS"))
	{
		updateUrgency();
	}
}

void Client::updateVisibility()
{
	QVector<xcb_atom_t> windowTypes = X11Support::getWindowPropertyAtomsArray(m_handle, "_NET_WM_WINDOW_TYPE");
	QVector<xcb_atom_t> windowStates = X11Support::getWindowPropertyAtomsArray(m_handle, "_NET_WM_STATE");

	// Show only regular windows in dock.
	// When no window type is set, assume it's normal window.
	m_visible = (windowTypes.size() == 0) || (windowTypes.size() == 1 && windowTypes[0] == X11Support::atom("_NET_WM_WINDOW_TYPE_NORMAL"));
	// Don't show window if requested explicitly in window states.
	if(windowStates.contains(X11Support::atom("_NET_WM_STATE_SKIP_TASKBAR")))
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
	m_name = X11Support::getWindowName(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->updateContent();
}

void Client::updateIcon()
{
	m_icon = X11Support::getWindowIcon(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->updateContent();
}

void Client::updateUrgency()
{
	m_isUrgent = X11Support::getWindowUrgency(m_handle);
	if(m_dockItem != NULL)
		m_dockItem->startAnimation();
}

DockApplet::DockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_dragging(false)
{
	// Register for notifications about window property changes.
	connect(X11Support::instance(), SIGNAL(windowPropertyChanged(xcb_window_t,xcb_atom_t)), this, SLOT(windowPropertyChanged(xcb_window_t,xcb_atom_t)));
}

DockApplet::~DockApplet()
{
	while(!m_clients.isEmpty())
	{
		xcb_window_t key = m_clients.begin().key();
		delete m_clients.begin().value();
		m_clients.remove(key);
	}

	while(!m_dockItems.isEmpty())
	{
		delete m_dockItems[m_dockItems.size() - 1];
	}
}

bool DockApplet::init()
{
	updateClientList();
	updateActiveWindow();

	for(int i = 0; i < m_dockItems.size(); i++)
		m_dockItems[i]->moveInstantly();

	return true;
}

void DockApplet::updateLayout()
{
	// TODO: Vertical orientation support.

	int freeSpace = m_size.width();
	int spaceForOneClient = (m_dockItems.size() > 0) ? freeSpace/m_dockItems.size() : 0;
	int currentPosition = 0;
	for(int i = 0; i < m_dockItems.size(); i++)
	{
		int spaceForThisClient = spaceForOneClient;
		static const int maxSpace = adjustHardcodedPixelSize(256);
		if(spaceForThisClient > maxSpace)
			spaceForThisClient = maxSpace;
		m_dockItems[i]->setTargetPosition(QPoint(currentPosition, 0));
		m_dockItems[i]->setTargetSize(QSize(spaceForThisClient - 4, m_size.height()));
		m_dockItems[i]->startAnimation();
		currentPosition += spaceForThisClient;
	}

	update();
}

void DockApplet::draggingStarted()
{
	m_dragging = true;
}

void DockApplet::draggingStopped()
{
	m_dragging = false;
	// Since we don't update it when dragging, we should do it now.
	updateClientList();
}

void DockApplet::moveItem(DockItem* dockItem, bool right)
{
	int currentIndex = m_dockItems.indexOf(dockItem);
	if(right)
	{
		if(currentIndex != (m_dockItems.size() - 1))
		{
			m_dockItems.remove(currentIndex);
			m_dockItems.insert(currentIndex + 1, dockItem);
			updateLayout();
		}
	}
	else
	{
		if(currentIndex != 0)
		{
			m_dockItems.remove(currentIndex);
			m_dockItems.insert(currentIndex - 1, dockItem);
			updateLayout();
		}
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

void DockApplet::registerDockItem(DockItem* dockItem)
{
	m_dockItems.append(dockItem);
	updateLayout();
	dockItem->moveInstantly();
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
	if(m_dragging)
		return; // Don't want new dock items to appear (or old to be removed) while rearranging them with drag and drop.

	QVector<xcb_window_t> windows = X11Support::getWindowPropertyWindowsArray(X11Support::rootWindow(), "_NET_CLIENT_LIST");

	// Handle new clients.
	for(int i = 0; i < windows.size(); i++)
	{
		if(!m_clients.contains(windows[i]))
		{
			// Skip our own windows.
			if(QWidget::find(windows[i]) != NULL)
				continue;

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
	m_activeWindow = X11Support::getWindowPropertyWindow(X11Support::rootWindow(), "_NET_ACTIVE_WINDOW");
}

void DockApplet::windowPropertyChanged(xcb_window_t window, xcb_atom_t atom)
{
	if(window == X11Support::rootWindow())
	{
		if(atom == X11Support::atom("_NET_CLIENT_LIST"))
		{
			updateClientList();
		}

		if(atom == X11Support::atom("_NET_ACTIVE_WINDOW"))
		{
			updateActiveWindow();
		}

		return;
	}

	if(m_clients.contains(window))
		m_clients[window]->windowPropertyChanged(atom);
}
