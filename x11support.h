#ifndef X11SUPPORT_H
#define X11SUPPORT_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtGui/QIcon>

class X11Support: public QObject
{
	Q_OBJECT
public:
	X11Support();
	~X11Support();

	static X11Support* instance()
	{
		return m_instance;
	}

	unsigned long rootWindow();
	unsigned long atom(const QString& name);

	void removeWindowProperty(unsigned long window, const QString& name);
	void setWindowPropertyCardinalArray(unsigned long window, const QString& name, const QVector<unsigned long>& values);
	void setWindowPropertyCardinal(unsigned long window, const QString& name, unsigned long value);
	unsigned long getWindowPropertyWindow(unsigned long window, const QString& name);
	QVector<unsigned long> getWindowPropertyWindowsArray(unsigned long window, const QString& name);
	QVector<unsigned long> getWindowPropertyAtomsArray(unsigned long window, const QString& name);
	QString getWindowPropertyUTF8String(unsigned long window, const QString& name);
	QString getWindowPropertyLatin1String(unsigned long window, const QString& name);
	QString getWindowName(unsigned long window);
	QIcon getWindowIcon(unsigned long window);
	void registerForWindowPropertyChanges(unsigned long window);
	void activateWindow(unsigned long window);
	void minimizeWindow(unsigned long window);
	bool makeSystemTray(unsigned long window);
	void freeSystemTray();

private:
	unsigned long systemTrayAtom();

	static X11Support* m_instance;
	QMap<QString, unsigned long> m_cachedAtoms;
};

#endif
