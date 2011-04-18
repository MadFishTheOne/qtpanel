#include "panelwindow.h"

#include <QtGui/QResizeEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

PanelWindow::PanelWindow()
	: m_dockMode(false), m_screen(0), m_horizontalAnchor(Center), m_verticalAnchor(Min), m_orientation(Horizontal)
{
	scene = new QGraphicsScene();
	scene->setBackgroundBrush(QBrush(Qt::darkGreen));

	view = new QGraphicsView(scene, this);
	view->setStyleSheet("border-style: none;");
	view->move(0, 0);

	resize(defaultWidth, defaultHeight);
}

PanelWindow::~PanelWindow()
{
	delete view;
	delete scene;
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
	view->resize(event->size());
}
