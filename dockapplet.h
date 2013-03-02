#ifndef DOCKAPPLET_H
#define DOCKAPPLET_H

#include <stdint.h>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtGui/QIcon>
#include <QtWidgets/QGraphicsItem>
#include "applet.h"

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;

class QGraphicsPixmapItem;
class TextGraphicsItem;
class DockApplet;
class Client;

// Represents a single item in a dock.
// There isn't one to one relationship between window (client) and dock item, that's why
// it's separate entity. One dock item can represent pinned launcher and one or more opened
// windows of that application.
class DockItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	DockItem(DockApplet* dockApplet);
	~DockItem();

	void updateContent();

	void addClient(Client* client);
	void removeClient(Client* client);

	void setTargetPosition(const QPoint& targetPosition);
	void setTargetSize(const QSize& targetSize);
	void moveInstantly();
	void startAnimation();

	const QVector<Client*>& clients() const
	{
		return m_clients;
	}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public slots:
	void animate();
	void close();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
	void updateClientsIconGeometry();
	bool isUrgent();

	QTimer* m_animationTimer;
	DockApplet* m_dockApplet;
	TextGraphicsItem* m_textItem;
	QGraphicsPixmapItem* m_iconItem;
	QVector<Client*> m_clients;
	QPoint m_position;
	QPoint m_targetPosition;
	QSize m_size;
	QSize m_targetSize;
	qreal m_highlightIntensity;
	qreal m_urgencyHighlightIntensity;
	bool m_dragging;
	QPointF m_mouseDownPosition;
	QPoint m_dragStartPosition;
};

// Used for tracking connected windows (X11 clients).
// Client may have it's DockItem, but not necessary (for example, special windows are not shown in dock).
class Client
{
public:
	Client(DockApplet* dockApplet, xcb_window_t handle);
	~Client();

	xcb_window_t handle() const
	{
		return m_handle;
	}

	bool isVisible()
	{
		return m_visible;
	}

	const QString& name() const
	{
		return m_name;
	}

	const QIcon& icon() const
	{
		return m_icon;
	}

	bool isUrgent() const
	{
		return m_isUrgent;
	}

	void windowPropertyChanged(xcb_atom_t atom);

private:
	void updateVisibility();
	void updateName();
	void updateIcon();
	void updateUrgency();

	DockApplet* m_dockApplet;
	xcb_window_t m_handle;
	QString m_name;
	QIcon m_icon;
	bool m_isUrgent;
	bool m_visible;
	DockItem* m_dockItem;
};

class DockApplet: public Applet
{
	Q_OBJECT
public:
	DockApplet(PanelWindow* panelWindow);
	~DockApplet();

	bool init();
	QSize desiredSize();

	void registerDockItem(DockItem* dockItem);
	void unregisterDockItem(DockItem* dockItem);

	DockItem* dockItemForClient(Client* client);

	void updateLayout();

	xcb_window_t activeWindow() const
	{
		return m_activeWindow;
	}

	void draggingStarted();
	void draggingStopped();
	void moveItem(DockItem* dockItem, bool right);

protected:
	void layoutChanged();

private slots:
	void windowPropertyChanged(xcb_window_t window, xcb_atom_t atom);

private:
	void updateClientList();
	void updateActiveWindow();

	QMap<xcb_window_t, Client*> m_clients;
	QVector<DockItem*> m_dockItems;
	xcb_window_t m_activeWindow;
	bool m_dragging;
};

#endif
