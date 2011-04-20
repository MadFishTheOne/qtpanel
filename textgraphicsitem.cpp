#include "textgraphicsitem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

TextGraphicsItem::TextGraphicsItem(QGraphicsItem* parent)
	: QGraphicsItem(parent)
{
}

TextGraphicsItem::~TextGraphicsItem()
{
}

void TextGraphicsItem::setColor(const QColor& color)
{
	m_color = color;
	update(boundingRect());
}

void TextGraphicsItem::setFont(const QFont& font)
{
	m_font = font;
	update(boundingRect());
}

void TextGraphicsItem::setText(const QString& text)
{
	m_text = text;
	update(boundingRect());
}

QRectF TextGraphicsItem::boundingRect() const
{
	QFontMetrics metrics(m_font);
	return metrics.boundingRect(m_text);
}

void TextGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(QPen(m_color));
	painter->setFont(m_font);
	painter->drawText(0, 0, m_text);
}
