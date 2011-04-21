#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include <QtCore/QVector>
#include <QtCore/QSet>
#include "applet.h"

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

private:
	QString m_name;
	QString m_exec;
	QString m_icon;
	QStringList m_categories;
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

private slots:
	void actionTriggered();

private:
	void updateDesktopFiles();
	void gatherDesktopFiles(const QString& path);

	QVector<DesktopFile> m_desktopFiles;
	TextGraphicsItem* m_textItem;
};

#endif
