#ifndef APPLET_H
#define APPLET_H

#include <QtCore/QObject>
#include <QtCore/QSize>

class PanelWindow;

class Applet: public QObject
{
	Q_OBJECT
public:
	Applet(PanelWindow* panelWindow);
	~Applet();
	virtual bool init() = 0;
	void setSize(const QSize& size);

signals:
	void desiredSizeChanged(const QSize& desiredSize);

protected:
	virtual void update() = 0;

	PanelWindow* m_panelWindow;
	QSize m_size;
};

#endif
