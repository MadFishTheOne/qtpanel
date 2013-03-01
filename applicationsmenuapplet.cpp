#include "applicationsmenuapplet.h"

#include <QtGui/QPixmap>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyle>
#include <QtWidgets/QGraphicsScene>
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include "desktopapplications.h"
#include "dpisupport.h"

int ApplicationsMenuStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
	if(metric == QStyle::PM_SmallIconSize)
		return adjustHardcodedPixelSize(32);
	else
		return QProxyStyle::pixelMetric(metric, option, widget);
}

SubMenu::SubMenu(QMenu* parent, const QString& title, const QString& category, const QString& icon)
{
	m_menu = new QMenu(parent); // Will be deleted automatically.
	m_menu->setStyle(parent->style());
	m_menu->setFont(parent->font());
	m_menu->setTitle(title);
	m_menu->setIcon(QIcon::fromTheme(icon));
	m_menu->menuAction()->setIconVisibleInMenu(true);
	m_category = category;
}

static const char* menuStyleSheet =
"QMenu { background-color: black; }\n"
"QMenu::item { height: %dpx; background-color: transparent; color: white; padding-left: %dpx; padding-right: %dpx; padding-top: %dpx; padding-bottom: %dpx; }\n"
"QMenu::item::selected { background-color: #606060; border-color: gray; }\n"
"QMenu::icon { left: %dpx; }\n";

ApplicationsMenuApplet::ApplicationsMenuApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_menuOpened(false)
{
	m_menu = new QMenu();
	m_menu->setStyle(&m_style);
	m_menu->setFont(m_panelWindow->font());
	m_menu->setStyleSheet(QString().sprintf(menuStyleSheet,
		adjustHardcodedPixelSize(36),
		adjustHardcodedPixelSize(38),
		adjustHardcodedPixelSize(20),
		adjustHardcodedPixelSize(2),
		adjustHardcodedPixelSize(2),
		adjustHardcodedPixelSize(2)
	));
	m_subMenus.append(SubMenu(m_menu, "Accessories", "Utility", "applications-accessories"));
	m_subMenus.append(SubMenu(m_menu, "Development", "Development", "applications-development"));
	m_subMenus.append(SubMenu(m_menu, "Education", "Education", "applications-science"));
	m_subMenus.append(SubMenu(m_menu, "Office", "Office", "applications-office"));
	m_subMenus.append(SubMenu(m_menu, "Graphics", "Graphics", "applications-graphics"));
	m_subMenus.append(SubMenu(m_menu, "Multimedia", "AudioVideo", "applications-multimedia"));
	m_subMenus.append(SubMenu(m_menu, "Games", "Game", "applications-games"));
	m_subMenus.append(SubMenu(m_menu, "Network", "Network", "applications-internet"));
	m_subMenus.append(SubMenu(m_menu, "System", "System", "preferences-system"));
	m_subMenus.append(SubMenu(m_menu, "Settings", "Settings", "preferences-desktop"));
	m_subMenus.append(SubMenu(m_menu, "Other", "Other", "applications-other"));

	m_textItem = new TextGraphicsItem(this);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
	m_textItem->setText("Applications");
}

ApplicationsMenuApplet::~ApplicationsMenuApplet()
{
	foreach(QAction* action, m_actions)
	{
		delete action;
	}

	delete m_textItem;
	delete m_menu;
}

bool ApplicationsMenuApplet::init()
{
	setInteractive(true);

	connect(DesktopApplications::instance(), SIGNAL(applicationUpdated(DesktopApplication)), this, SLOT(applicationUpdated(DesktopApplication)));
	connect(DesktopApplications::instance(), SIGNAL(applicationRemoved(QString)), this, SLOT(applicationRemoved(QString)));

	QList<DesktopApplication> apps = DesktopApplications::instance()->applications();
	foreach(const DesktopApplication& app, apps)
	applicationUpdated(app);

	return true;
}

QSize ApplicationsMenuApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().size().width() + 16, m_textItem->boundingRect().size().height());
}

void ApplicationsMenuApplet::clicked()
{
	m_menuOpened = true;
	animateHighlight();

	m_menu->move(localToScreen(QPoint(0, m_size.height())));
	m_menu->exec();

	m_menuOpened = false;
	animateHighlight();
}

void ApplicationsMenuApplet::layoutChanged()
{
	m_textItem->setPos(8, m_panelWindow->textBaseLine());
}

bool ApplicationsMenuApplet::isHighlighted()
{
	return m_menuOpened || Applet::isHighlighted();
}

void ApplicationsMenuApplet::actionTriggered()
{
	DesktopApplications::instance()->launch(static_cast<QAction*>(sender())->data().toString());
}

void ApplicationsMenuApplet::applicationUpdated(const DesktopApplication& app)
{
	applicationRemoved(app.path());

	if(app.isNoDisplay())
		return;

	QAction* action = new QAction(m_menu);
	action->setIconVisibleInMenu(true);
	action->setData(app.path());
	action->setText(app.name());
	action->setIcon(QIcon(QPixmap::fromImage(app.iconImage())));

	connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));

	// Add to relevant menu.
	int subMenuIndex = m_subMenus.size() - 1; // By default put it in "Other".
	for(int i = 0; i < m_subMenus.size() - 1; i++) // Without "Other".
	{
		if(app.categories().contains(m_subMenus[i].category()))
		{
			subMenuIndex = i;
			break;
		}
	}

	QMenu* menu = m_subMenus[subMenuIndex].menu();
	QList<QAction*> actions = menu->actions();
	QAction* before = NULL;
	for(int i = 0; i < actions.size(); i++)
	{
		if(actions[i]->text().compare(action->text(), Qt::CaseInsensitive) > 0)
		{
			before = actions[i];
			break;
		}
	}

	if(menu->actions().isEmpty())
	{
		QList<QAction*> actions = m_menu->actions();
		QAction* before = NULL;
		for(int i = 0; i < actions.size(); i++)
		{
			if(actions[i]->text().compare(menu->title(), Qt::CaseInsensitive) > 0)
			{
				before = actions[i];
				break;
			}
		}

		m_menu->insertMenu(before, menu);
	}

	menu->insertAction(before, action);


	m_actions[app.path()] = action;
}

void ApplicationsMenuApplet::applicationRemoved(const QString& path)
{
	if(m_actions.contains(path))
	{
		delete m_actions[path];
		m_actions.remove(path);
	}

	for(int i = 0; i < m_subMenus.size(); i++)
	{
		if(m_subMenus[i].menu()->actions().isEmpty())
			m_menu->removeAction(m_subMenus[i].menu()->menuAction());
	}
}
