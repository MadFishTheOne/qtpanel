#ifndef DOCKAPPLET_H
#define DOCKAPPLET_H

#include <QtCore/QVector>
#include "applet.h"

class DockApplet;

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

private:
	DockApplet* m_dockApplet;
	unsigned long m_handle;
	QString m_name;
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

protected:
	void layoutChanged();

private slots:
	void clientListChanged();

private:
	QVector<Client*> m_clients;
};

#endif
