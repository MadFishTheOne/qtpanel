#ifndef X11SUPPORT_H
#define X11SUPPORT_H

#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

class X11Support: public QObject
{
	Q_OBJECT
public:
	X11Support();
	~X11Support();

	void onX11Event(XEvent* event);

	static X11Support* instance()
	{
		return m_instance;
	}

	static unsigned long rootWindow();
	static unsigned long atom(const QString& name);

	static void removeWindowProperty(unsigned long window, const QString& name);
	static void setWindowPropertyCardinalArray(unsigned long window, const QString& name, const QVector<unsigned long>& values);
	static void setWindowPropertyCardinal(unsigned long window, const QString& name, unsigned long value);
	static void setWindowPropertyVisualId(unsigned long window, const QString& name, unsigned long value);
	static unsigned long getWindowPropertyCardinal(unsigned long window, const QString& name);
	static unsigned long getWindowPropertyWindow(unsigned long window, const QString& name);
	static QVector<unsigned long> getWindowPropertyWindowsArray(unsigned long window, const QString& name);
	static QVector<unsigned long> getWindowPropertyAtomsArray(unsigned long window, const QString& name);
	static QString getWindowPropertyUTF8String(unsigned long window, const QString& name);
	static QString getWindowPropertyLatin1String(unsigned long window, const QString& name);
	static QString getWindowName(unsigned long window);
	static QIcon getWindowIcon(unsigned long window);
	static bool getWindowUrgency(unsigned long window);
	static void registerForWindowPropertyChanges(unsigned long window);
	static void registerForTrayIconUpdates(unsigned long window);
	static void activateWindow(unsigned long window);
	static void minimizeWindow(unsigned long window);
	static void closeWindow(unsigned long window);
	static void destroyWindow(unsigned long window);
	static void killClient(unsigned long window);
	static bool makeSystemTray(unsigned long window);
	static void freeSystemTray();
	static unsigned long getARGBVisualId();
	static void redirectWindow(unsigned long window);
	static void unredirectWindow(unsigned long window);
	static QPixmap getWindowPixmap(unsigned long window);
	static void resizeWindow(unsigned long window, int width, int height);
	static void moveWindow(unsigned long window, int x, int y);
	static void mapWindow(unsigned long window);
	static void reparentWindow(unsigned long window, unsigned long parent);

signals:
	void windowClosed(unsigned long window);
	void windowReconfigured(unsigned long window, int x, int y, int width, int height);
	void windowDamaged(unsigned long window);
	void windowPropertyChanged(unsigned long window, unsigned long atom);
	void clientMessageReceived(unsigned long window, unsigned long atom, void* data);

private:
	static unsigned long systemTrayAtom();

	static X11Support* m_instance;
	int m_damageEventBase;
	QMap<QString, unsigned long> m_cachedAtoms;
};

#endif
