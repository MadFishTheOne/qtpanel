#ifndef APPLET_H
#define APPLET_H

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QGraphicsItem>

class Applet;

class AppletGraphicsItem: public QGraphicsItem
{
public:
	AppletGraphicsItem(Applet* applet);
	~AppletGraphicsItem();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
	Applet* m_applet;
	qreal m_highlightIntensity;
};

class PanelWindow;

class Applet: public QObject
{
	Q_OBJECT
public:
	Applet(PanelWindow* panelWindow);
	~Applet();

	virtual bool init();

	void setPosition(const QPoint& position);

	const QSize& size() const
	{
		return m_size;
	}

	void setSize(const QSize& size);

	virtual QSize desiredSize() = 0;

	PanelWindow* panelWindow()
	{
		return m_panelWindow;
	}

	AppletGraphicsItem* appletItem()
	{
		return m_appletItem;
	}

public slots:
	virtual void clicked();

protected:
	virtual void layoutChanged();
	QPoint localToScreen(const QPoint& point);

	PanelWindow* m_panelWindow;
	AppletGraphicsItem* m_appletItem;
	QPoint m_position;
	QSize m_size;
};

#endif
