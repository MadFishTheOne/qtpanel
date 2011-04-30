#ifndef CLOCKAPPLET_H
#define CLOCKAPPLET_H

#include "applet.h"

class QTimer;
class TextGraphicsItem;

class ClockApplet: public Applet
{
	Q_OBJECT
public:
	ClockApplet(PanelWindow* panelWindow);
	~ClockApplet();

	bool init();
	QSize desiredSize();

protected:
	void layoutChanged();

private slots:
	void updateContent();

private:
	void scheduleUpdate();

	QTimer* m_timer;
	QString m_text;
	TextGraphicsItem* m_textItem;
};

#endif
