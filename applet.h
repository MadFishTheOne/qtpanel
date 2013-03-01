#ifndef APPLET_H
#define APPLET_H

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtWidgets/QGraphicsItem>

class PanelWindow;

class Applet: public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	Applet(PanelWindow* panelWindow);
	~Applet();

	virtual bool init();

	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

	const QSize& size() const
	{
		return m_size;
	}

	virtual QSize desiredSize() = 0;

	PanelWindow* panelWindow()
	{
		return m_panelWindow;
	}

	void setInteractive(bool interactive);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public slots:
	void animateHighlight();
	virtual void clicked();

protected:
	virtual void layoutChanged();
	QPoint localToScreen(const QPoint& point);

	virtual bool isHighlighted();

	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

	PanelWindow* m_panelWindow;
	QPoint m_position;
	QSize m_size;
	bool m_interactive;
	qreal m_highlightIntensity;
};

#endif
