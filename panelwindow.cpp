#include "panelwindow.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

PanelWindow::PanelWindow()
	: m_orientation(Horizontal)
{
}

PanelWindow::~PanelWindow()
{
}

void PanelWindow::setLocation(int screen, Side side)
{
	m_orientation = ((side == Top) || (side == Bottom)) ? Horizontal : Vertical;

	QRect screenGeometry = QApplication::desktop()->screenGeometry(screen);

	static const int horizontalPanelHeight = 48;
	static const int verticalPanelWidth = 48;

	switch(side)
	{
	case Top:
		move(screenGeometry.topLeft());
		resize(screenGeometry.width(), horizontalPanelHeight);
		break;
	case Bottom:
		move(screenGeometry.bottomLeft() - QPoint(0, horizontalPanelHeight));
		resize(screenGeometry.width(), horizontalPanelHeight);
		break;
	case Left:
		move(screenGeometry.topLeft());
		resize(verticalPanelWidth, screenGeometry.height());
		break;
	case Right:
		move(screenGeometry.topRight() - QPoint(verticalPanelWidth, 0));
		resize(verticalPanelWidth, screenGeometry.height());
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	setAttribute(Qt::WA_X11NetWmWindowTypeDock);
}
