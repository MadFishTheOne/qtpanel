#ifndef TRAYAPPLET_H
#define TRAYAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QSize>
#include "applet.h"

class QX11EmbedContainer;
class TrayApplet;

class TrayItem: public QObject
{
	Q_OBJECT
public:
	TrayItem(TrayApplet* trayApplet, unsigned long window);
	~TrayItem();

	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

private slots:
	void clientClosed();
	void clientIsEmbedded();

private:
	QSize m_size;
	TrayApplet* m_trayApplet;
	unsigned long m_window;
	QX11EmbedContainer* m_container;
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

private:
	void updateLayout();

	bool m_initialized;
	QVector<TrayItem*> m_trayItems;
};

#endif
