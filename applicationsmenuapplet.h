#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtGui/QAction>
#include <QtGui/QPlastiqueStyle>
#include "applet.h"

class ApplicationsMenuStyle: public QPlastiqueStyle
{
	Q_OBJECT
public:
	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
};

class DesktopFile
{
public:
	bool init(const QString& fileName);

	const QString& name() const
	{
		return m_name;
	}

	const QString& exec() const
	{
		return m_exec;
	}

	const QString& icon() const
	{
		return m_icon;
	}

	const QStringList& categories() const
	{
		return m_categories;
	}

	void setAction(QAction* action)
	{
		m_action = action;
	}

	QAction* action()
	{
		return m_action;
	}

private:
	QString m_name;
	QString m_exec;
	QString m_icon;
	QStringList m_categories;
	QAction* m_action;
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

private:
	void updateDesktopFiles();
	void gatherDesktopFiles(const QString& path);
	void desktopFileAdded(const QString& fileName);

	ApplicationsMenuStyle m_style;
	TextGraphicsItem* m_textItem;
	bool m_menuOpened;
	QMenu* m_menu;
	QVector<SubMenu> m_subMenus;
	QMap<QString, DesktopFile> m_desktopFiles;
};

#endif
