#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QtGui/QWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>

class PanelWindow: public QWidget
{
	Q_OBJECT
public:
	static const int defaultWidth = 512;
	static const int defaultHeight = 48;

	PanelWindow();
	~PanelWindow();

	enum Anchor
	{
		Min,
		Center,
		Max,
	};

	enum Orientation
	{
		Horizontal,
		Vertical,
	};

	void setDockMode(bool dockMode);
	void setScreen(int screen);
	void setHorizontalAnchor(Anchor horizontalAnchor);
	void setVerticalAnchor(Anchor verticalAnchor);
	void setOrientation(Orientation orientation);
	void updatePosition();

	void resizeEvent(QResizeEvent* event);

private:
	bool m_dockMode;
	int m_screen;
	Anchor m_horizontalAnchor;
	Anchor m_verticalAnchor;
	Orientation m_orientation;

	QGraphicsScene* scene;
	QGraphicsView* view;
};

#endif
