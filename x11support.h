#ifndef X11SUPPORT_H
#define X11SUPPORT_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QObject>

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
	QVector<unsigned long> getWindowPropertyWindowsArray(unsigned long window, const QString& name);
	QString getWindowPropertyUTF8String(unsigned long window, const QString& name);
	QString getWindowPropertyLatin1String(unsigned long window, const QString& name);
	QString getWindowName(unsigned long window);

private:
	static X11Support* m_instance;
	QMap<QString, unsigned long> m_cachedAtoms;
};

#endif
