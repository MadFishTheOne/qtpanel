#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QtGui/QWidget>

class QGraphicsScene;
class QGraphicsView;
class Applet;

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

	bool init();

	void setDockMode(bool dockMode);
	void setScreen(int screen);
	void setHorizontalAnchor(Anchor horizontalAnchor);
	void setVerticalAnchor(Anchor verticalAnchor);
	void setOrientation(Orientation orientation);
	void updatePosition();

	QGraphicsScene* scene()
	{
		return m_scene;
	}

	void resizeEvent(QResizeEvent* event);

private:
	bool m_dockMode;
	int m_screen;
	Anchor m_horizontalAnchor;
	Anchor m_verticalAnchor;
	bool m_orientation;

	QGraphicsScene* m_scene;
	QGraphicsView* m_view;
	Applet* m_applet;
};

#endif
