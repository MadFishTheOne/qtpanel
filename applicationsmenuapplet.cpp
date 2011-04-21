#include "applicationsmenuapplet.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtGui/QMenu>
#include <QtGui/QStyle>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelwindow.h"

#include <stdio.h>

bool DesktopFile::init(const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	while(!in.atEnd())
	{
		QString line = in.readLine();
		if(line[0] == '[' || line[0] == '#')
			continue;
		QStringList list = line.split('=');
		if(list.size() < 2)
			continue;
		QString key = list[0];
		QString value = list[1];
		if(key == "NoDisplay" && value == "true")
			return false;
		if(key == "Name")
			m_name = value;
		if(key == "Exec")
			m_exec = value;
		if(key == "Icon")
			m_icon = value;
		if(key == "Categories")
			m_categories = value.split(";", QString::SkipEmptyParts);
	}
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

struct SubMenu
{
	QMenu* m_menu;
	QString m_category;

	SubMenu()
	{
	}

	SubMenu(QMenu* parent, const QString& title, const QString& category, const QString& icon)
	{
		m_menu = new QMenu(parent); // Will be deleted automatically.
		m_menu->setTitle(title);
		m_menu->setIcon(QIcon::fromTheme(icon));
		m_menu->menuAction()->setIconVisibleInMenu(true);
		m_category = category;
	}
};

void ApplicationsMenuApplet::clicked()
{
	QMenu menu;
	// Submenus.
	QVector<SubMenu> subMenus;
	subMenus.append(SubMenu(&menu, "Accessories", "Utility", "applications-accessories"));
	subMenus.append(SubMenu(&menu, "Development", "Development", "applications-development"));
	subMenus.append(SubMenu(&menu, "Education", "Education", "applications-science"));
	subMenus.append(SubMenu(&menu, "Office", "Office", "applications-office"));
	subMenus.append(SubMenu(&menu, "Graphics", "Graphics", "applications-graphics"));
	subMenus.append(SubMenu(&menu, "Multimedia", "AudioVideo", "applications-multimedia"));
	subMenus.append(SubMenu(&menu, "Games", "Game", "applications-games"));
	subMenus.append(SubMenu(&menu, "Network", "Network", "applications-internet"));
	subMenus.append(SubMenu(&menu, "System", "System", "preferences-system"));
	subMenus.append(SubMenu(&menu, "Settings", "Settings", "preferences-desktop"));
	subMenus.append(SubMenu(&menu, "Other", "Other", "applications-other"));

	// Applications.
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
		action->setData(m_desktopFiles[i].exec());
		connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));

		// Add to relevant menu.
		bool subMenuFound = false;
		for(int k = 0; k < subMenus.size() - 1; k++) // Without "Other".
		{
			if(m_desktopFiles[i].categories().contains(subMenus[k].m_category))
			{
				subMenus[k].m_menu->addAction(action);
				subMenuFound = true;
				break;
			}
		}
		if(!subMenuFound)
			subMenus[subMenus.size() - 1].m_menu->addAction(action); // Add to "Other" if no category matches.
	}

	// Add non-empty submenus.
	for(int i = 0; i < subMenus.size(); i++)
	{
		if(subMenus[i].m_menu->actions().size() > 0)
			menu.addMenu(subMenus[i].m_menu);
	}

	menu.move(localToScreen(QPoint(0, m_size.height())));
	menu.exec();
}

void ApplicationsMenuApplet::layoutChanged()
{
	m_textItem->setPos(8, m_panelWindow->textBaseLine());
}

void ApplicationsMenuApplet::actionTriggered()
{
	QString exec = static_cast<QAction*>(sender())->data().toString();

	// Don't need arguments from desktop file here.
	int spacePos = exec.indexOf(' ');
	if(spacePos != -1)
		exec.resize(spacePos);

	QProcess process;
	process.startDetached(exec);
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
