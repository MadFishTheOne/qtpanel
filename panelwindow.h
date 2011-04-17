#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QtGui/QWidget>

class PanelWindow: public QWidget
{
	Q_OBJECT
public:
	PanelWindow();
	~PanelWindow();

	enum Side
	{
		Top,
		Bottom,
		Left,
		Right,
	};

	enum Orientation
	{
		Horizontal,
		Vertical,
	};

	void setLocation(int screen, Side side);

private:
	Orientation m_orientation;
};

#endif
