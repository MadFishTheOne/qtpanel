#ifndef TRAYAPPLET_H
#define TRAYAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QSize>
#include "applet.h"

class TrayApplet;

class TrayItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	TrayItem(TrayApplet* trayApplet, unsigned long window);
	~TrayItem();

	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	unsigned long window() const
	{
		return m_window;
	}

private:
	QSize m_size;
	TrayApplet* m_trayApplet;
	unsigned long m_window;
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

protected:
	void layoutChanged();

private slots:
	void clientMessageReceived(unsigned long window, unsigned long atom, void* data);
	void windowClosed(unsigned long window);
	void windowReconfigured(unsigned long window, int x, int y, int width, int height);
	void windowDamaged(unsigned long window);

private:
	void updateLayout();

	bool m_initialized;
	QVector<TrayItem*> m_trayItems;
};

#endif
