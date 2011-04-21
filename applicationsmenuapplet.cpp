#include "applicationsmenuapplet.h"

#include <QtGui/QMenu>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelwindow.h"

ApplicationsMenuApplet::ApplicationsMenuApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_textItem = new TextGraphicsItem();
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
	m_textItem->setText("Applications");
	m_panelWindow->scene()->addItem(m_textItem);
}

ApplicationsMenuApplet::~ApplicationsMenuApplet()
{
	m_panelWindow->scene()->removeItem(m_textItem);
	delete m_textItem;
}

bool ApplicationsMenuApplet::init()
{
	return true;
}

QSize ApplicationsMenuApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().size().width() + 16, m_textItem->boundingRect().size().height());
}

void ApplicationsMenuApplet::clicked()
{
	QMenu menu;
	menu.addAction("Action 1");
	menu.addAction("Action 2");
	menu.addAction("Action 3");
	menu.move(localToScreen(QPoint(0, m_size.height())));
	menu.exec();
}

void ApplicationsMenuApplet::layoutChanged()
{
	m_textItem->setPos(8, m_panelWindow->textBaseLine());
}
