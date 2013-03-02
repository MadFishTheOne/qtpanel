#ifndef X11SUPPORT_H
#define X11SUPPORT_H

#include <stdint.h>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QAbstractNativeEventFilter>
#include <QtGui/QIcon>

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;

struct xcb_connection_t;
struct xcb_screen_t;

class X11Support: public QObject, public QAbstractNativeEventFilter
{
	Q_OBJECT
public:
	X11Support();
	~X11Support();

	static X11Support* instance() { return m_instance; }
	static xcb_connection_t* connection() { return m_instance->m_connection; }
	static xcb_window_t rootWindow() { return m_instance->m_rootWindow; }
	static xcb_atom_t atom(const QString& name);

	static void removeWindowProperty(xcb_window_t window, const QString& name);
	static void setWindowPropertyCardinalArray(xcb_window_t window, const QString& name, const QVector<uint32_t>& values);
	static void setWindowPropertyCardinal(xcb_window_t window, const QString& name, uint32_t value);
	static void setWindowPropertyAtom(xcb_window_t window, const QString& name, const QString& value);
	static uint32_t getWindowPropertyCardinal(xcb_window_t window, const QString& name);
	static uint32_t getWindowPropertyWindow(xcb_window_t window, const QString& name);
	static QVector<xcb_window_t> getWindowPropertyWindowsArray(xcb_window_t window, const QString& name);
	static QVector<xcb_atom_t> getWindowPropertyAtomsArray(xcb_window_t window, const QString& name);
	static QString getWindowPropertyUTF8String(xcb_window_t window, const QString& name);
	static QString getWindowPropertyLatin1String(xcb_window_t window, const QString& name);
	static QString getWindowName(xcb_window_t window);
	static QIcon getWindowIcon(xcb_window_t window);
	static bool getWindowUrgency(xcb_window_t window);
	static void registerForWindowPropertyChanges(xcb_window_t window);
	static bool registerForTrayIconUpdates(xcb_window_t window);
	static void activateWindow(xcb_window_t window);
	static void minimizeWindow(xcb_window_t window);
	static void closeWindow(xcb_window_t window);
	static bool makeSystemTray(xcb_window_t window);
	static void freeSystemTray();
	static void resizeWindow(xcb_window_t window, int width, int height);
	static void moveWindow(xcb_window_t window, int x, int y);
	static void mapWindow(xcb_window_t window);
	static void reparentWindow(xcb_window_t window, xcb_window_t parent);
	static void setWindowBackgroundBlack(xcb_window_t window);

signals:
	void windowClosed(xcb_window_t window);
	void windowReconfigured(xcb_window_t window, int x, int y, int width, int height);
	void windowPropertyChanged(xcb_window_t window, xcb_atom_t atom);
	void clientMessageReceived(xcb_window_t window, xcb_atom_t atom, const uint32_t* data);

private:
	static xcb_atom_t systemTrayAtom() { return m_instance->m_systemTrayAtom; }

	virtual bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);

	static X11Support* m_instance;
	xcb_connection_t* m_connection;
	xcb_window_t m_rootWindow;
	xcb_screen_t* m_screen;
	int m_screenNumber;
	xcb_atom_t m_systemTrayAtom;
	QMap<QString, xcb_atom_t> m_cachedAtoms;
};

#endif
