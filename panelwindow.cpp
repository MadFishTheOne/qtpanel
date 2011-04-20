#include "panelwindow.h"

#include <QtGui/QResizeEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include "x11support.h"

#include "demoapplet.h"
#include "spacerapplet.h"
#include "clockapplet.h"

PanelWindowGraphicsItem::PanelWindowGraphicsItem(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
	setZValue(-1.0); // Background.
}

PanelWindowGraphicsItem::~PanelWindowGraphicsItem()
{
}

QRectF PanelWindowGraphicsItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_panelWindow->width(), m_panelWindow->height());
}

void PanelWindowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(0, 0, 0, 128));
	painter->drawRect(boundingRect());

	static const int borderThickness = 3;
	if(m_panelWindow->verticalAnchor() == PanelWindow::Min)
	{
		QLinearGradient gradient(0.0, m_panelWindow->height() - borderThickness, 0.0, m_panelWindow->height());
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0, QColor(255, 255, 255, 0));
		gradient.setColorAt(1, QColor(255, 255, 255, 128));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, m_panelWindow->height() - borderThickness, m_panelWindow->width(), borderThickness);
	}
	else
	{
		QLinearGradient gradient(0.0, 0.0, 0.0, borderThickness);
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0, QColor(255, 255, 255, 128));
		gradient.setColorAt(1, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, 0.0, m_panelWindow->width(), borderThickness);
	}
}

PanelWindow::PanelWindow()
	: m_dockMode(false), m_screen(0), m_horizontalAnchor(Center), m_verticalAnchor(Min), m_orientation(Horizontal), m_layoutPolicy(Normal)
{
	setStyleSheet("background-color: transparent");
	setAttribute(Qt::WA_TranslucentBackground);

	m_scene = new QGraphicsScene();
	m_scene->setBackgroundBrush(QBrush(Qt::NoBrush));

	m_panelItem = new PanelWindowGraphicsItem(this);
	m_scene->addItem(m_panelItem);

	m_view = new QGraphicsView(m_scene, this);
	m_view->setStyleSheet("border-style: none;");
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//m_view->setRenderHint(QPainter::Antialiasing);
	m_view->move(0, 0);

	m_applets.append(new DemoApplet(this));
	m_applets.append(new DemoApplet(this));
	m_applets.append(new DemoApplet(this));
	m_applets.append(new SpacerApplet(this));
	m_applets.append(new ClockApplet(this));

	resize(defaultWidth, defaultHeight);
}

PanelWindow::~PanelWindow()
{
	for(int i = 0; i < m_applets.size(); i++)
		delete m_applets[i];
	delete m_view;
	delete m_panelItem;
	delete m_scene;
}

bool PanelWindow::init()
{
	for(int i = 0; i < m_applets.size(); i++)
	{
		if(!m_applets[i]->init())
			return false;
	}
}

void PanelWindow::setDockMode(bool dockMode)
{
	m_dockMode = dockMode;
	setAttribute(Qt::WA_X11NetWmWindowTypeDock, m_dockMode);
	if(!m_dockMode)
	{
		// No need to reserve space anymore.
		X11Support::instance()->removeWindowProperty(winId(), "_NET_WM_STRUT");
		X11Support::instance()->removeWindowProperty(winId(), "_NET_WM_STRUT_PARTIAL");
	}
	updateLayout();
	updatePosition();
}

void PanelWindow::setScreen(int screen)
{
	m_screen = screen;
	updateLayout();
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

void PanelWindow::setLayoutPolicy(LayoutPolicy layoutPolicy)
{
	m_layoutPolicy = layoutPolicy;
	updateLayout();
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
		x = screenGeometry.right() - width() + 1;
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
		y = screenGeometry.bottom() - height() + 1;
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	move(x, y);

	// Update reserved space.
	if(m_dockMode)
	{
		QVector<unsigned long> values; // Values for setting _NET_WM_STRUT_PARTIAL property.
		values.fill(0, 12);
		switch(m_horizontalAnchor)
		{
		case Min:
			values[0] = width();
			values[4] = y;
			values[5] = y + height();
			break;
		case Max:
			values[1] = width();
			values[6] = y;
			values[7] = y + height();
			break;
		default:
			break;
		}
		switch(m_verticalAnchor)
		{
		case Min:
			values[2] = height();
			values[8] = x;
			values[9] = x + width();
			break;
		case Max:
			values[3] = height();
			values[10] = x;
			values[11] = x + width();
			break;
		default:
			break;
		}
		X11Support::instance()->setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT_PARTIAL", values);
		values.resize(4);
		X11Support::instance()->setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT", values);
	}

	// Update "blur behind" hint.
	QVector<unsigned long> values;
	values.resize(4);
	values[0] = 0;
	values[1] = 0;
	values[2] = width();
	values[3] = height();
	X11Support::instance()->setWindowPropertyCardinalArray(winId(), "_KDE_NET_WM_BLUR_BEHIND_REGION", values);
}

void PanelWindow::resizeEvent(QResizeEvent* event)
{
	m_view->resize(event->size());
	m_view->setSceneRect(0, 0, event->size().width(), event->size().height());
	updateLayout();
	updatePosition();
}

void PanelWindow::updateLayout()
{
	// TODO: Vertical orientation support.

	if(m_layoutPolicy != Normal && !m_dockMode)
	{
		int desiredSize = 0;
		if(m_layoutPolicy == AutoSize)
		{
			for(int i = 0; i < m_applets.size(); i++)
			{
				if(m_applets[i]->desiredSize().width() >= 0)
					desiredSize += m_applets[i]->desiredSize().width();
				else
					desiredSize += 64; // Spacer applets don't really make sense on auto-size panel.
			}
		}
		if(m_layoutPolicy == FillSpace)
		{
			QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);
			desiredSize = screenGeometry.width();
		}

		if(desiredSize != width())
			resize(desiredSize, height());
	}

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
