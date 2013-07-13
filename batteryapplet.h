#ifndef BATTERYAPPLET_H
#define BATTERYAPPLET_H

#include "applet.h"
#include "textgraphicsitem.h"
#include <QTimer>

class BatteryApplet: public Applet
{
    Q_OBJECT
public:
    BatteryApplet(PanelWindow* panelWindow);
    ~BatteryApplet();

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
