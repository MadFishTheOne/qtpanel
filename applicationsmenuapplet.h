#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include <QtCore/QVector>
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

private:
	QString m_name;
	QString m_exec;
	QString m_icon;
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

private:
	void updateDesktopFiles();

	QVector<DesktopFile> m_desktopFiles;
	TextGraphicsItem* m_textItem;
};

#endif
