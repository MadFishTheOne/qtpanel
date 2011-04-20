#ifndef APPLET_H
#define APPLET_H

#include <QtCore/QObject>
#include <QtCore/QRect>

class PanelWindow;

class Applet: public QObject
{
	Q_OBJECT
public:
	Applet(PanelWindow* panelWindow);
	~Applet();
	virtual bool init();
	void setRect(const QRect& rect);
	virtual QSize desiredSize() = 0;

protected slots:
	virtual void update();

protected:
	PanelWindow* m_panelWindow;
	QRect m_rect;
};

#endif
