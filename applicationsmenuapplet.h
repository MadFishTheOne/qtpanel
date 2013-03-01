#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtWidgets/QAction>
#include <QtWidgets/QProxyStyle>
#include "applet.h"

class ApplicationsMenuStyle: public QProxyStyle
{
	Q_OBJECT
public:
	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
};

class SubMenu
{
public:
	SubMenu()
	{
	}

	SubMenu(QMenu* parent, const QString& title, const QString& category, const QString& icon);

	QMenu* menu()
	{
		return m_menu;
	}

	const QString& category() const
	{
		return m_category;
	}

private:
	QMenu* m_menu;
	QString m_category;
};

class TextGraphicsItem;
class DesktopApplication;

class ApplicationsMenuApplet: public Applet
{
	Q_OBJECT
public:
	ApplicationsMenuApplet(PanelWindow* panelWindow);
	~ApplicationsMenuApplet();

	bool init();
	QSize desiredSize();
	void clicked();

protected:
	void layoutChanged();
	bool isHighlighted();

private slots:
	void actionTriggered();
	void applicationUpdated(const DesktopApplication& app);
	void applicationRemoved(const QString& path);

private:
	ApplicationsMenuStyle m_style;
	TextGraphicsItem* m_textItem;
	bool m_menuOpened;
	QMenu* m_menu;
	QList<SubMenu> m_subMenus;
	QMap<QString, QAction*> m_actions;
};

#endif
