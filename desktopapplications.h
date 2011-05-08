#ifndef DESKTOPAPPLICATIONS_H
#define DESKTOPAPPLICATIONS_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtGui/QImage>

class DesktopApplication
{
public:
	DesktopApplication()
		: m_isNoDisplay(false)
	{
	}

	bool init(const QString& path);

	bool exists() const;

	bool needUpdate() const;

	void launch() const;

	const bool isNoDisplay() const { return m_isNoDisplay; }
	const QString& path() const { return m_path; }
	const QString& name() const { return m_name; }
	const QString& iconName() const { return m_iconName; }
	void setIconImage(const QImage& iconImage) { m_iconImage = iconImage; }
	const QImage& iconImage() const { return m_iconImage; }
	const QStringList& categories() const { return m_categories; }

private:
	QString m_path;
	QDateTime m_lastUpdated;

	bool m_isNoDisplay;
	QString m_name;
	QString m_exec;
	QString m_iconName;
	QImage m_iconImage;
	QStringList m_categories;
};

Q_DECLARE_METATYPE(DesktopApplication)

class QDir;
class QTimer;
class QFileSystemWatcher;

class DesktopApplications: public QThread
{
	Q_OBJECT
public:
	DesktopApplications();
	~DesktopApplications();

	static DesktopApplications* instance() { return m_instance; }

	QList<DesktopApplication> applications();
	DesktopApplication applicationFromPath(const QString& path);
	void launch(const QString& path);

signals:
	void applicationUpdated(const DesktopApplication& app);
	void applicationRemoved(const QString& path);

protected:
	void run();

private slots:
	void directoryChanged(const QString& path);
	void fileChanged(const QString& path);
	void refresh();

private:
	void traverse(const QDir& dir);

	static DesktopApplications* m_instance;

	QTimer* m_updateTimer;
	QFileSystemWatcher* m_watcher;
	QMutex m_applicationsMutex;
	QMap<QString, DesktopApplication> m_applications;
	QMutex m_tasksMutex;
	QWaitCondition m_tasksWaitCondition;
	bool m_abortWorker;
	QStringList m_fileTasks;
	QStringList m_imageTasks;
};

#endif
