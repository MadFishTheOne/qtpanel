#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QtCore/QVector>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>

class QFont;
class QGraphicsScene;
class QGraphicsView;
class Applet;
class PanelWindow;

class PanelWindowGraphicsItem: public QGraphicsItem
{
public:
	PanelWindowGraphicsItem(PanelWindow* panelWindow);
	~PanelWindowGraphicsItem();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	PanelWindow* m_panelWindow;
};

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

	enum LayoutPolicy
	{
		Normal,
		AutoSize,
		FillSpace,
	};

	bool init();

	bool dockMode() const
	{
		return m_dockMode;
	}

	void setDockMode(bool dockMode);

	int screen() const
	{
		return m_screen;
	}

	void setScreen(int screen);

	Anchor horizontalAnchor() const
	{
		return m_horizontalAnchor;
	}

	void setHorizontalAnchor(Anchor horizontalAnchor);

	Anchor verticalAnchor() const
	{
		return m_verticalAnchor;
	}

	void setVerticalAnchor(Anchor verticalAnchor);

	Orientation orientation() const
	{
		return m_orientation;
	}

	void setOrientation(Orientation orientation);

	LayoutPolicy layoutPolicy() const
	{
		return m_layoutPolicy;
	}

	void setLayoutPolicy(LayoutPolicy layoutPolicy);

	void updatePosition();

	const QFont& font() const
	{
		return m_font;
	}

	int textBaseLine();

	QGraphicsScene* scene()
	{
		return m_scene;
	}

	void resizeEvent(QResizeEvent* event);

	void updateLayout();

private:
	bool m_dockMode;
	int m_screen;
	Anchor m_horizontalAnchor;
	Anchor m_verticalAnchor;
	Orientation m_orientation;
	LayoutPolicy m_layoutPolicy;

	QFont m_font;
	QGraphicsScene* m_scene;
	QGraphicsView* m_view;
	PanelWindowGraphicsItem* m_panelItem;
	QVector<Applet*> m_applets;
};

#endif
