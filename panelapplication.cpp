#include "panelapplication.h"

#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include "iconloader.h"
#include "x11support.h"
#include "dpisupport.h"
#include "desktopapplications.h"
#include "ui_panelapplicationsettings.h"

PanelApplication* PanelApplication::m_instance = NULL;

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_instance = this;

	m_defaultIconThemeName = QIcon::themeName();

	setOrganizationName("madfish");
	setApplicationName("qtpanel");

	m_iconLoader = new IconLoader();
	m_x11support = new X11Support();
	m_desktopApplications = new DesktopApplications();
}

PanelApplication::~PanelApplication()
{
	reset();
	delete m_desktopApplications;
	delete m_x11support;
	delete m_iconLoader;

	m_instance = NULL;
}

void PanelApplication::showConfigurationDialog()
{
	QDialog dialog;
	Ui::PanelApplicationSettings settingsUi;
	settingsUi.setupUi(&dialog);
	settingsUi.fontName->setText(m_fontName);
	settingsUi.iconThemeName->setText(m_iconThemeName);
	settingsUi.verticalPosition->setCurrentIndex(m_verticalAnchor == PanelWindow::Max ? 1 : 0);
	if(dialog.exec() == QDialog::Accepted)
	{
		QSettings settings;
		settings.setValue("fontName", settingsUi.fontName->text());
		settings.setValue("iconThemeName", settingsUi.iconThemeName->text());
		settings.setValue("verticalPosition", settingsUi.verticalPosition->currentText());

		// Don't want to delete objects right now (because we're called from those objects), schedule it for later.
		QTimer::singleShot(1, this, SLOT(reinit()));
	}
}

void PanelApplication::reinit()
{
	reset();
	init();
}

void PanelApplication::init()
{
	QSettings settings;
	setFontName(settings.value("fontName", "default").toString());
	setIconThemeName(settings.value("iconThemeName", "default").toString());
	QString verticalPosition = settings.value("verticalPosition", "Top").toString();
	if(verticalPosition == "Top")
		m_verticalAnchor = PanelWindow::Min;
	else if(verticalPosition == "Bottom")
		m_verticalAnchor = PanelWindow::Max;

	PanelWindow* panelWindow = new PanelWindow();
	panelWindow->resize(adjustHardcodedPixelSize(128), adjustHardcodedPixelSize(32));
	panelWindow->setLayoutPolicy(PanelWindow::FillSpace);
	panelWindow->setVerticalAnchor(m_verticalAnchor);
	panelWindow->setDockMode(true);
	panelWindow->init();
	panelWindow->show();

	m_panelWindows.append(panelWindow);
}

void PanelApplication::reset()
{
	for(int i = 0; i < m_panelWindows.size(); i++)
	{
		delete m_panelWindows[i];
	}
	m_panelWindows.clear();
}

void PanelApplication::setFontName(const QString& fontName)
{
	m_fontName = fontName;
	if(m_fontName != "default")
	{
		// Parse font name. Example: "Droid Sans 11".
		int lastSpacePos = m_fontName.lastIndexOf(' ');
		// Should have at least one space, otherwise string is malformed, keep default in that case.
		if(lastSpacePos != -1)
		{
			int fontSize = m_fontName.mid(lastSpacePos).toInt();
			QString fontFamily = m_fontName;
			fontFamily.truncate(lastSpacePos);
			m_panelFont = QFont(fontFamily, fontSize);
		}
	}
	else
	{
		m_panelFont = QApplication::font();
	}
}

void PanelApplication::setIconThemeName(const QString& iconThemeName)
{
	m_iconThemeName = iconThemeName;
	if(m_iconThemeName != "default")
		QIcon::setThemeName(m_iconThemeName);
	else
		QIcon::setThemeName(m_defaultIconThemeName);
}
