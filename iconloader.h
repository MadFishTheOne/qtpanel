#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QtCore/QMutex>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtGui/QImage>

// Unfortunately, Qt icon theme support is not flexible enough.
// I need to load icon images in another thread, and this is not possible with Qt implementation.
// Also, I would like to specify what theme to use when calling load function, instead of specifying it globally.
// Reinventing the wheel here...

class IconTheme
{
public:
	void init(const QString& themeName);
	QImage loadIcon(const QString& iconName, int size);

	const QStringList& inheritedThemes()
	{
		return m_inheritedThemes;
	}

private:
	struct IconDirectory
	{
		QString m_path;
		int m_size;
		bool m_scalable;
	};

	bool loadIconFromDirectory(QImage& result, const IconDirectory& iconDir, const QString& fileName);

	QString m_themeName;
	QStringList m_inheritedThemes;
	QList<IconDirectory> m_iconDirs;
};

class IconLoader
{
public:
	IconLoader();
	~IconLoader();

	static IconLoader* instance()
	{
		return m_instance;
	}

	const QStringList& iconSearchPaths()
	{
		return m_iconSearchPaths;
	}

	QImage loadIcon(const QString& themeName, const QString& iconName, int size);

private:
	QImage loadIconFromTheme(const QString& themeName, const QString& iconName, int size);

	static IconLoader* m_instance;
	QStringList m_iconSearchPaths;
	QMutex m_iconThemesMutex;
	QMap<QString, IconTheme> m_iconThemes;
};

#endif
