#ifndef PANELAPPLICATION_H
#define PANELAPPLICATION_H

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include "panelwindow.h"

class X11Support;

class PanelApplication: public QApplication
{
	Q_OBJECT
public:
	PanelApplication(int& argc, char** argv);
	~PanelApplication();

	static PanelApplication* instance()
	{
		return m_instance;
	}

	bool x11EventFilter(XEvent* event);

	void init();
	void saveSettings();

	void reset();

	void setFontName(const QString& fontName);
	void setIconThemeName(const QString& iconThemeName);

	const QFont& panelFont() const
	{
		return m_panelFont;
	}

public slots:
	void showConfigurationDialog();

private slots:
	void reinit();

private:
	static PanelApplication* m_instance;
	X11Support* m_x11support;

	QString m_fontName;
	QString m_iconThemeName;
	QString m_defaultIconThemeName;
	QFont m_panelFont;
	QVector<PanelWindow*> m_panelWindows;
};

#endif
