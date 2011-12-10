#include "iconloader.h"

#include <stdlib.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>

struct IconDirectory
{
	QString m_path;
	int m_size;
	bool m_scalable;
};

void IconTheme::init(const QString& themeName)
{
	m_themeName = themeName;

	foreach(const QString& searchPath, IconLoader::instance()->iconSearchPaths())
	{
		QFile file(searchPath + "/" + m_themeName + "/index.theme");

		if(!file.exists())
			continue;

		if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
			continue;

		QTextStream in(&file);
		QString context;
		while(!in.atEnd())
		{
			QString line = in.readLine();
			if(line.isEmpty())
				continue;
			if(line[0] == '#')
				continue;
			if(line[0] == '[')
			{
				if(line.size() < 3)
					continue;
				context = line.mid(1, line.size() - 2);
				if(context != "Icon Theme")
				{
					IconDirectory dir;
					dir.m_path = context;
					dir.m_size = 0;
					dir.m_scalable = false;
					m_iconDirs.append(dir);
				}
			}

			QStringList list = line.split('=');
			if(list.size() < 2)
				continue;
			QString key = list[0];
			QString value = list[1];

			if(key == "Inherits")
				m_inheritedThemes = value.split(',');

			if(key == "Size")
			{
				m_iconDirs.last().m_size = value.toInt();
			}

			if(key == "Type")
			{
				if(value.compare("Scalable", Qt::CaseInsensitive) == 0)
					m_iconDirs.last().m_scalable = true;
			}
		}
	}
}

bool IconTheme::loadIconFromDirectory(QImage& result, const IconDirectory& iconDir, const QString& fileName)
{
	foreach(const QString& searchPath, IconLoader::instance()->iconSearchPaths())
	{
		QString iconFileName = searchPath + "/" + m_themeName + "/" + iconDir.m_path + "/" + fileName;
		if(!QFile::exists(iconFileName))
			continue;
		result.load(iconFileName);
		if(!result.isNull())
			return true;
	}
	return false;
}

QImage IconTheme::loadIcon(const QString& iconName, int size)
{
	QString fileName = iconName;
	if(!fileName.endsWith(".png"))
		fileName.append(".png");

	QImage result;

	foreach(const IconDirectory& iconDir, m_iconDirs)
	{
		if(!iconDir.m_scalable && iconDir.m_size == size)
		{
			if(loadIconFromDirectory(result, iconDir, fileName))
				return result;
		}
	}

	foreach(const IconDirectory& iconDir, m_iconDirs)
	{
		if(!iconDir.m_scalable && iconDir.m_size > size)
		{
			if(loadIconFromDirectory(result, iconDir, fileName))
				return result;
		}
	}

	foreach(const IconDirectory& iconDir, m_iconDirs)
	{
		if(!iconDir.m_scalable)
		{
			if(loadIconFromDirectory(result, iconDir, fileName))
				return result;
		}
	}

	return result;
}

IconLoader* IconLoader::m_instance = NULL;

IconLoader::IconLoader()
{
	m_instance = this;

	m_iconSearchPaths.append(QString(getenv("HOME")) + "/.icons");

	QString xdgDataDirs;
	char* xdgDataDirsEnv = getenv("XDG_DATA_DIRS");
	if(xdgDataDirsEnv != NULL)
		xdgDataDirs = xdgDataDirsEnv;
	else
		xdgDataDirs = "/usr/local/share/:/usr/share/";

	QStringList dirs = xdgDataDirs.split(':');
	foreach(const QString& dir, dirs)
	{
		m_iconSearchPaths.append(dir + "icons");
	}
}

IconLoader::~IconLoader()
{
	m_instance = NULL;
}

QImage IconLoader::loadIcon(const QString& themeName, const QString& iconName, int size)
{
	QImage result = loadIconFromTheme(themeName, iconName, size);

	if(result.isNull())
		result.load("/usr/share/pixmaps/" + iconName);

	if(result.isNull())
		result.load(iconName);

	if(!result.isNull() && (result.width() != size || result.height() != size))
		result = result.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	return result;
}

QImage IconLoader::loadIconFromTheme(const QString& themeName, const QString& iconName, int size)
{
	if(!m_iconThemes.contains(themeName))
	{
		m_iconThemesMutex.lock();
		m_iconThemes[themeName].init(themeName);
		m_iconThemesMutex.unlock();
	}

	QImage result = m_iconThemes[themeName].loadIcon(iconName, size);

	if(result.isNull() && themeName != "hicolor")
	{
		QStringList inheritedThemes = m_iconThemes[themeName].inheritedThemes();
		if(inheritedThemes.empty())
			inheritedThemes.append("hicolor");
		foreach(const QString& inheritedTheme, inheritedThemes)
		{
			result = loadIconFromTheme(inheritedTheme, iconName, size);
			if(!result.isNull())
				break;
		}
	}

	return result;
}
