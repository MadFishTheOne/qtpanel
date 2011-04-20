#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>

class TextGraphicsItem: public QGraphicsItem
{
public:
	void setColor(const QColor& color);
	void setFont(const QFont& font);
	void setText(const QString& text);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	QColor m_color;
	QFont m_font;
	QString m_text;
};

#endif
