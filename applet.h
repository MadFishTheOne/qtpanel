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
	QSize size() const
	{
		return m_size;
	}
	void setSize(const QSize& size);
	virtual QSize desiredSize() = 0;

protected:
	virtual void layoutChanged();

	PanelWindow* m_panelWindow;
	AppletGraphicsItem* m_appletItem;
	QSize m_size;
};

#endif
