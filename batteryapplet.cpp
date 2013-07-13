#include "batteryapplet.h"
#include "battery.h"
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include <QtWidgets/QGraphicsScene>


BatteryApplet::BatteryApplet(PanelWindow* panelWindow)
    : Applet(panelWindow)
{
    m_timer = new QTimer();
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
    m_textItem = new TextGraphicsItem(this);
    m_textItem->setColor(Qt::white);
    m_textItem->setFont(m_panelWindow->font());
}

BatteryApplet::~BatteryApplet()
{
    delete m_textItem;
    delete m_timer;
}

bool BatteryApplet::init()
{
    updateContent();

    setInteractive(true);
    return true;
}

void BatteryApplet::layoutChanged()
{
    m_textItem->setPos((m_size.width() - m_textItem->boundingRect().size().width())/2.0, m_panelWindow->textBaseLine());
}

void BatteryApplet::updateContent()
{
    Battery::Status battery_status(Battery::status());
    int energy_now = Battery::current_energy();
    int energy_full = Battery::total_capacity();

    float cap = (float) energy_now / (float) energy_full * 100;
    QString status;

    if (battery_status == Battery::CHARGING)
        status = "Charging";
    else
        status = "Discharging";

    m_text = QString("Battery: %1% (%2)").arg(cap, 0, 'f', 0).arg(status);
    m_textItem->setText(m_text);
    update();
    scheduleUpdate();
}

QSize BatteryApplet::desiredSize()
{
    return QSize(m_textItem->boundingRect().width() + 16, m_textItem->boundingRect().height() + 16);
}

void BatteryApplet::scheduleUpdate()
{
    m_timer->setInterval(1000);
    m_timer->start();
}

