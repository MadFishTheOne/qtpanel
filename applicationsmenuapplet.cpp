#include "applicationsmenuapplet.h"

#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtGui/QMenu>
#include <QtGui/QStyle>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelwindow.h"

bool DesktopFile::init(const QString& fileName)
{
	QSettings settings(fileName, QSettings::IniFormat);
	settings.beginGroup("Desktop Entry");
	if(settings.value("NoDisplay") == "true")
		return false;
	m_name = settings.value("Name").toString();
	m_exec = settings.value("Exec").toString();
	m_icon = settings.value("Icon").toString();
	settings.endGroup();
	return true;
}

ApplicationsMenuApplet::ApplicationsMenuApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{
	m_textItem = new TextGraphicsItem();
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
	m_textItem->setText("Applications");
	m_panelWindow->scene()->addItem(m_textItem);
}

ApplicationsMenuApplet::~ApplicationsMenuApplet()
{
	m_panelWindow->scene()->removeItem(m_textItem);
	delete m_textItem;
}

bool ApplicationsMenuApplet::init()
{
	updateDesktopFiles();
	return true;
}

QSize ApplicationsMenuApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().size().width() + 16, m_textItem->boundingRect().size().height());
}

#include <stdio.h>

void ApplicationsMenuApplet::clicked()
{
	QMenu menu;
	for(int i = 0; i < m_desktopFiles.size(); i++)
	{
		QAction* action = new QAction(&menu); // Will be deleted automatically.
		action->setText(m_desktopFiles[i].name());
		QIcon icon = QIcon::fromTheme(m_desktopFiles[i].icon());
		if(icon.isNull())
		{
			if(m_desktopFiles[i].icon().contains('/'))
				icon = QIcon(m_desktopFiles[i].icon());
			else
				icon = QIcon("/usr/share/pixmaps/" + m_desktopFiles[i].icon());
		}
		int iconSize = menu.style()->pixelMetric(QStyle::PM_SmallIconSize);
		if(!icon.availableSizes().empty())
		{
			if(!icon.availableSizes().contains(QSize(iconSize, iconSize)))
			{
				QPixmap pixmap = icon.pixmap(256); // Any big size here is fine (at least for now).
				QPixmap scaledPixmap = pixmap.scaled(QSize(iconSize, iconSize));
				icon = QIcon(scaledPixmap);
			}
		}
		action->setIcon(icon);
		action->setIconVisibleInMenu(true);
		menu.addAction(action);
	}
	menu.move(localToScreen(QPoint(0, m_size.height())));
	menu.exec();
}

void ApplicationsMenuApplet::layoutChanged()
{
	m_textItem->setPos(8, m_panelWindow->textBaseLine());
}

void ApplicationsMenuApplet::updateDesktopFiles()
{
	m_desktopFiles.clear();
	QDir dir("/usr/share/applications");
	QStringList list = dir.entryList(QStringList("*.desktop"));
	foreach(const QString& fileName, list)
	{
		DesktopFile file;
		if(file.init(dir.path() + '/' + fileName))
			m_desktopFiles.append(file);
	}
}
