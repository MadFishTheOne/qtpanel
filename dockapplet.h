#ifndef DOCKAPPLET_H
#define DOCKAPPLET_H

#include <QtCore/QVector>
#include <QtGui/QGraphicsItem>
#include "applet.h"

class TextGraphicsItem;
class DockApplet;
class Client;

class ClientGraphicsItem: public QGraphicsItem
{
public:
	ClientGraphicsItem(Client* client);
	~ClientGraphicsItem();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	Client* m_client;
};

class Client
{
public:
	Client(DockApplet* dockApplet, unsigned long handle);
	~Client();

	unsigned long handle() const
	{
		return m_handle;
	}

	// Separate from destructor, as we would like to animate removal, so actual
	// destruction will happen later.
	void removed();

	bool isVisible()
	{
		return m_visible;
	}

	QSize desiredSize();
	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

	const QSize& size() const
	{
		return m_size;
	}

	void updateLayout();
	void updateName();
	void updateVisibility();

private:
	DockApplet* m_dockApplet;
	ClientGraphicsItem* m_clientItem;
	unsigned long m_handle;
	QString m_name;
	TextGraphicsItem* m_textItem;
	QSize m_size;
	bool m_visible;
};

class DockApplet: public Applet
{
	Q_OBJECT
public:
	DockApplet(PanelWindow* panelWindow);
	~DockApplet();

	bool init();
	QSize desiredSize();

	void registerClient(Client* client);
	void unregisterClient(Client* client);

	void updateLayout();

protected:
	void layoutChanged();

private slots:
	void clientListChanged();

private:
	QVector<Client*> m_clients;
};

#endif
