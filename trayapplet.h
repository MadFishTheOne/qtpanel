#ifndef TRAYAPPLET_H
#define TRAYAPPLET_H

#include <stdint.h>
#include <QtCore/QVector>
#include <QtCore/QSize>
#include "applet.h"

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;

class TrayApplet;

class TrayItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	TrayItem(TrayApplet* trayApplet, xcb_window_t window);
	~TrayItem();

	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	xcb_window_t window() const
	{
		return m_window;
	}

private:
	QSize m_size;
	TrayApplet* m_trayApplet;
	xcb_window_t m_window;
};

class TrayApplet: public Applet
{
	Q_OBJECT
public:
	TrayApplet(PanelWindow* panelWindow);
	~TrayApplet();

	bool init();

	QSize desiredSize();

	void registerTrayItem(TrayItem* trayItem);
	void unregisterTrayItem(TrayItem* trayItem);

	int iconSize() const { return m_iconSize; }

protected:
	void layoutChanged();

private slots:
	void clientMessageReceived(xcb_window_t window, xcb_atom_t atom, void* data);
	void windowClosed(xcb_window_t window);
	void windowReconfigured(xcb_window_t window, int x, int y, int width, int height);
	void windowDamaged(xcb_window_t window);

private:
	void updateLayout();

	bool m_initialized;
	QVector<TrayItem*> m_trayItems;
	int m_iconSize;
	int m_spacing;
};

#endif
