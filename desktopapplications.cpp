#include "desktopapplications.h"

#include <QtCore/QTimer>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QMutexLocker>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtGui/QIcon>
#include "iconloader.h"
#include "dpisupport.h"

bool DesktopApplication::init(const QString& path)
{
	m_path = path;

	QFile file(m_path);

	if(!file.exists())
		return false;

	QFileInfo fileInfo(file);
	m_lastUpdated = fileInfo.lastModified();

	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	while(!in.atEnd())
	{
		QString line = in.readLine();
		if(line[0] == '[')
		{
			if(line.contains("Desktop Entry"))
				continue;
			else
				break; // We only process "Desktop Entry" here.
		}
		if(line[0] == '#')
			continue;
		QStringList list = line.split('=');
		if(list.size() < 2)
			continue;
		QString key = list[0];
		QString value = list[1];
		if(key == "NoDisplay" && value == "true")
			m_isNoDisplay = true;
		if(key == "Name")
			m_name = value;
		if(key == "Exec")
			m_exec = value;
		if(key == "Icon")
		{
			m_iconName = value;
		}
		if(key == "Categories")
			m_categories = value.split(";", QString::SkipEmptyParts);
	}

	return true;
}

bool DesktopApplication::exists() const
{
	return QFile(m_path).exists();
}

bool DesktopApplication::needUpdate() const
{
	return m_lastUpdated != QFileInfo(m_path).lastModified();
}

void DesktopApplication::launch() const
{
	QString exec = m_exec;

	// Handle special arguments.
	for(;;)
	{
		int argPos = exec.indexOf('%');
		if(argPos == -1)
			break;
		// For now, just remove them.
		int spacePos = exec.indexOf(' ', argPos);
		if(spacePos == -1)
			exec.resize(argPos);
		else
			exec.remove(argPos, spacePos - argPos);
	}

	exec = exec.trimmed();
	QStringList args = exec.split(' ');
	QString process = args[0];
	args.removeAt(0);
	QProcess::startDetached(process, args, getenv("HOME"));
}

DesktopApplications* DesktopApplications::m_instance = NULL;

DesktopApplications::DesktopApplications()
	: m_abortWorker(false)
{
	m_instance = this;

	qRegisterMetaType<DesktopApplication>();

	m_updateTimer = new QTimer();
	m_updateTimer->setSingleShot(true);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(refresh()));

	m_watcher = new QFileSystemWatcher();
	connect(m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged(QString)));
	connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

	// Start worker and trigger initial update.
	start(QThread::IdlePriority);
	refresh();
}

DesktopApplications::~DesktopApplications()
{
	m_abortWorker = true;
	m_tasksWaitCondition.wakeOne();
	wait();

	delete m_watcher;
	delete m_updateTimer;

	m_instance = NULL;
}

QList<DesktopApplication> DesktopApplications::applications()
{
	QMutexLocker lock(&m_applicationsMutex);
	return m_applications.values();
}

DesktopApplication DesktopApplications::applicationFromPath(const QString& path)
{
	QMutexLocker lock(&m_applicationsMutex);
	return m_applications[path];
}

void DesktopApplications::launch(const QString& path)
{
	QMutexLocker lock(&m_applicationsMutex);
	m_applications[path].launch();
}

void DesktopApplications::run()
{
	forever
	{
		// Extract next task.
		bool isImageTask = false;
		QString path;
		{
			QMutexLocker lock(&m_tasksMutex);
			if(m_fileTasks.isEmpty() && m_imageTasks.isEmpty())
				m_tasksWaitCondition.wait(&m_tasksMutex);
			if(m_abortWorker)
				return;
			if(!m_fileTasks.isEmpty())
			{
				path = m_fileTasks.first();
				m_fileTasks.removeFirst();
			}
			else
			{
				isImageTask = true;
				path = m_imageTasks.first();
				m_imageTasks.removeFirst();
			}
		}

		if(!isImageTask)
		{
			// File task.
			m_applicationsMutex.lock();
			bool needUpdate = (!m_applications.contains(path)) || (m_applications[path].needUpdate());
			m_applicationsMutex.unlock();

			if(needUpdate)
			{
				DesktopApplication app;
				if(app.init(path))
				{
					m_applicationsMutex.lock();
					m_applications[path] = app;
					emit applicationUpdated(app);
					m_applicationsMutex.unlock();

					m_tasksMutex.lock();
					if(!m_imageTasks.contains(path))
						m_imageTasks.append(path);
					m_tasksMutex.unlock();
				}
			}
		}
		else
		{
			// Image task.
			m_applicationsMutex.lock();
			QString iconName;
			if(m_applications.contains(path))
				iconName = m_applications[path].iconName();
			m_applicationsMutex.unlock();

			if(!iconName.isEmpty())
			{
				QImage iconImage = IconLoader::instance()->loadIcon(QIcon::themeName(), iconName, adjustHardcodedPixelSize(32));

				m_applicationsMutex.lock();
				if(m_applications.contains(path))
					m_applications[path].setIconImage(iconImage);
				emit applicationUpdated(m_applications[path]);
				m_applicationsMutex.unlock();
			}
		}
	}
}

void DesktopApplications::directoryChanged(const QString& path)
{
	m_updateTimer->stop();
	m_updateTimer->start();
}

void DesktopApplications::fileChanged(const QString& path)
{
	QMutexLocker lock(&m_tasksMutex);
	if(!m_fileTasks.contains(path))
		m_fileTasks.append(path);
}

void DesktopApplications::refresh()
{
	m_tasksMutex.lock();

	m_fileTasks.clear();
	m_imageTasks.clear();

	if(!m_watcher->directories().isEmpty())
		m_watcher->removePaths(m_watcher->directories());
	if(!m_watcher->files().isEmpty())
		m_watcher->removePaths(m_watcher->files());

	QString xdgDataDirs;
	char* xdgDataDirsEnv = getenv("XDG_DATA_DIRS");
	if(xdgDataDirsEnv != NULL)
		xdgDataDirs = xdgDataDirsEnv;
	else
		xdgDataDirs = "/usr/local/share/:/usr/share/";

	QStringList dirs = xdgDataDirs.split(':');

	foreach(const QString& path, dirs)
	{
		QDir dir(path);
		if(dir.exists())
			traverse(QDir(dir.absoluteFilePath("applications")));
	}

	QStringList removeList;
	foreach(const DesktopApplication& app, m_applications)
	{
		if(!app.exists())
			removeList.append(app.path());
	}

	foreach(const QString& path, removeList)
	{
		m_applications.remove(path);
		emit applicationRemoved(path);
	}

	m_tasksMutex.unlock();
	m_tasksWaitCondition.wakeOne();
}

void DesktopApplications::traverse(const QDir& dir)
{
	if(!dir.exists())
		return;

	m_watcher->addPath(dir.canonicalPath());

	QFileInfoList fileInfos = dir.entryInfoList(QStringList("*.desktop"), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
	foreach(const QFileInfo& fileInfo, fileInfos)
	{
		if(fileInfo.isDir())
		{
			traverse(fileInfo.canonicalFilePath());
		}
		else
		{
			m_watcher->addPath(fileInfo.canonicalFilePath());
			m_fileTasks.append(fileInfo.canonicalFilePath());
		}
	}
}
