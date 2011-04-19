#include "panelwindow.h"

#include <QtGui/QResizeEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include "demoapplet.h"
#include "spacerapplet.h"

PanelWindow::PanelWindow()
	: m_dockMode(false), m_screen(0), m_horizontalAnchor(Center), m_verticalAnchor(Min), m_orientation(false)
{
	m_scene = new QGraphicsScene();
	m_scene->setBackgroundBrush(QBrush(Qt::darkGreen));

	m_view = new QGraphicsView(m_scene, this);
	m_view->setStyleSheet("border-style: none;");
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->move(0, 0);

	m_applets.append(new DemoApplet(this));
	m_applets.append(new DemoApplet(this));
	m_applets.append(new DemoApplet(this));
	m_applets.append(new SpacerApplet(this));
	m_applets.append(new DemoApplet(this));
	m_applets.append(new DemoApplet(this));

	resize(defaultWidth, defaultHeight);
}

PanelWindow::~PanelWindow()
{
	for(int i = 0; i < m_applets.size(); i++)
		delete m_applets[i];
	delete m_view;
	delete m_scene;
}

bool PanelWindow::init()
{
	for(int i = 0; i < m_applets.size(); i++)
	{
		if(!m_applets[i]->init())
			return false;
//		m_applets[i]->setRect(QRect(32*i, 0, 32, 32));
	}
}

void PanelWindow::setDockMode(bool dockMode)
{
	m_dockMode = dockMode;
	setAttribute(Qt::WA_X11NetWmWindowTypeDock, m_dockMode);
	updatePosition();
}

void PanelWindow::setScreen(int screen)
{
	m_screen = screen;
	updatePosition();
}

void PanelWindow::setHorizontalAnchor(Anchor horizontalAnchor)
{
	m_horizontalAnchor = horizontalAnchor;
	updatePosition();
}

void PanelWindow::setVerticalAnchor(Anchor verticalAnchor)
{
	m_verticalAnchor = verticalAnchor;
	updatePosition();
}

void PanelWindow::setOrientation(Orientation orientation)
{
	m_orientation = orientation;
}

void PanelWindow::updatePosition()
{
	if(!m_dockMode)
		return;

	QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);

	int x;

	switch(m_horizontalAnchor)
	{
	case Min:
		x = screenGeometry.left();
		break;
	case Center:
		x = (screenGeometry.left() + screenGeometry.right() - width())/2;
		break;
	case Max:
		x = screenGeometry.right() - width();
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	int y;

	switch(m_verticalAnchor)
	{
	case Min:
		y = screenGeometry.top();
		break;
	case Center:
		y = (screenGeometry.top() + screenGeometry.bottom() - height())/2;
		break;
	case Max:
		y = screenGeometry.bottom() - height();
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	move(x, y);
}

void PanelWindow::resizeEvent(QResizeEvent* event)
{
	m_view->resize(event->size());
	m_view->setSceneRect(0, 0, event->size().width(), event->size().height());
	updateLayout();
}

void PanelWindow::updateLayout()
{
	// TODO: Vertical orientation support.

	// Get total amount of space available for "spacer" applets (that take all available free space).
	int freeSpace = width();
	int numSpacers = 0;
	for(int i = 0; i < m_applets.size(); i++)
	{
		if(m_applets[i]->desiredSize().width() >= 0)
			freeSpace -= m_applets[i]->desiredSize().width();
		else
			numSpacers++;
	}
	int spaceForOneSpacer = freeSpace/numSpacers;

	// Calculate rectangles for each applet.
	int spacePos = 0;
	for(int i = 0; i < m_applets.size(); i++)
	{
		QPoint appletPosition(spacePos, 0);
		QSize appletSize = m_applets[i]->desiredSize();

		if(appletSize.width() < 0)
		{
			if(numSpacers > 1)
			{
				appletSize.setWidth(spaceForOneSpacer);
				freeSpace -= spaceForOneSpacer;
				numSpacers--;
			}
			else
			{
				appletSize.setWidth(freeSpace);
				freeSpace = 0;
				numSpacers--;
			}
		}

		if(appletSize.height() < 0 || appletSize.height() > height())
		{
			appletSize.setHeight(height());
		}
		else
		{
			appletPosition.setY((height() - appletSize.height())/2);
		}

		m_applets[i]->setRect(QRect(appletPosition, appletSize));

		spacePos += appletSize.width();
	}
}
