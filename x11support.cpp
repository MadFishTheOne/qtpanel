#include "x11support.h"

#include <QtGui/QApplication>
#include <QtGui/QX11Info>
#include <QtGui/QImage>

// Keep all the X11 stuff with scary defines below normal headers.
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static XErrorHandler oldX11ErrorHandler = NULL;

static int x11errorHandler(Display* display, XErrorEvent* error)
{
	if(error->error_code == BadWindow)
		return 0; // This usually happens when querying property on a window that's already gone. That's OK.

	return (*oldX11ErrorHandler)(display, error);
}

X11Support* X11Support::m_instance = NULL;

X11Support::X11Support()
{
	m_instance = this;

	oldX11ErrorHandler = XSetErrorHandler(x11errorHandler);
}

X11Support::~X11Support()
{
	m_instance = NULL;
}

unsigned long X11Support::rootWindow()
{
	return QX11Info::appRootWindow();
}

unsigned long X11Support::atom(const QString& name)
{
	if(!m_instance->m_cachedAtoms.contains(name))
		m_instance->m_cachedAtoms[name] = XInternAtom(QX11Info::display(), name.toLatin1().data(), False);
	return m_instance->m_cachedAtoms[name];
}

void X11Support::removeWindowProperty(unsigned long window, const QString& name)
{
	XDeleteProperty(QX11Info::display(), window, atom(name));
}

void X11Support::setWindowPropertyCardinalArray(unsigned long window, const QString& name, const QVector<unsigned long>& values)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(values.data()), values.size());
}

void X11Support::setWindowPropertyCardinal(unsigned long window, const QString& name, unsigned long value)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&value), 1);
}

template<class T>
static bool getWindowPropertyHelper(unsigned long window, unsigned long atom, unsigned long type, int& numItems, T*& data)
{
	Atom retType;
	int retFormat;
	unsigned long numItemsTemp;
	unsigned long bytesLeft;
	if(XGetWindowProperty(QX11Info::display(), window, atom, 0, 0x7FFFFFFF, False, type, &retType, &retFormat, &numItemsTemp, &bytesLeft, reinterpret_cast<unsigned char**>(&data)) != Success)
		return false;
	numItems = numItemsTemp;
	if(numItems == 0)
		return false;
	return true;
}

unsigned long X11Support::getWindowPropertyWindow(unsigned long window, const QString& name)
{
	int numItems;
	unsigned long* data;
	unsigned long value = 0;
	if(!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return value;
	value = data[0];
	XFree(data);
	return value;
}

QVector<unsigned long> X11Support::getWindowPropertyWindowsArray(unsigned long window, const QString& name)
{
	int numItems;
	unsigned long* data;
	QVector<unsigned long> values;
	if(!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return values;
	for(int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;
}

QVector<unsigned long> X11Support::getWindowPropertyAtomsArray(unsigned long window, const QString& name)
{
	int numItems;
	unsigned long* data;
	QVector<unsigned long> values;
	if(!getWindowPropertyHelper(window, atom(name), XA_ATOM, numItems, data))
		return values;
	for(int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;
}

QString X11Support::getWindowPropertyUTF8String(unsigned long window, const QString& name)
{
	int numItems;
	char* data;
	QString value;
	if(!getWindowPropertyHelper(window, atom(name), atom("UTF8_STRING"), numItems, data))
		return value;
	value = QString::fromUtf8(data);
	XFree(data);
	return value;
}

QString X11Support::getWindowPropertyLatin1String(unsigned long window, const QString& name)
{
	int numItems;
	char* data;
	QString value;
	if(!getWindowPropertyHelper(window, atom(name), XA_STRING, numItems, data))
		return value;
	value = QString::fromLatin1(data);
	XFree(data);
	return value;
}

QString X11Support::getWindowName(unsigned long window)
{
	QString result = getWindowPropertyUTF8String(window, "_NET_WM_VISIBLE_NAME");
	if(result.isEmpty())
		result = getWindowPropertyUTF8String(window, "_NET_WM_NAME");
	if(result.isEmpty())
		result = getWindowPropertyLatin1String(window, "WM_NAME");
	if(result.isEmpty())
		result = "<Unknown>";
	return result;
}

QIcon X11Support::getWindowIcon(unsigned long window)
{
	int numItems;
	unsigned long* rawData;
	QIcon icon;
	if(!getWindowPropertyHelper(window, atom("_NET_WM_ICON"), XA_CARDINAL, numItems, rawData))
		return icon;
	unsigned long* data = rawData;
	while(numItems > 0)
	{
		int width = static_cast<int>(data[0]);
		int height = static_cast<int>(data[1]);
		data += 2;
		numItems -= 2;
		QImage image(width, height, QImage::Format_ARGB32);
		for(int i = 0; i < height; i++)
		{
			for(int k = 0; k < width; k++)
			{
				image.setPixel(k, i, static_cast<unsigned int>(data[i*width + k]));
			}
		}
		data += width*height;
		numItems -= width*height;
		icon.addPixmap(QPixmap::fromImage(image));
	}
	XFree(rawData);
	return icon;
}

void X11Support::registerForWindowPropertyChanges(unsigned long window)
{
	XSelectInput(QX11Info::display(), window, PropertyChangeMask);
}

void X11Support::activateWindow(unsigned long window)
{
	XWindowChanges wc;
	wc.stack_mode = Above;
	XConfigureWindow(QX11Info::display(), window, CWStackMode, &wc);

	// Apparently, KWin won't bring window to top with configure request,
	// so we also need to ask it politely by sending a message.
	XClientMessageEvent event;
	event.type = ClientMessage;
	event.window = window;
	event.message_type = atom("_NET_ACTIVE_WINDOW");
	event.format = 32;
	event.data.l[0] = 2;
	event.data.l[1] = CurrentTime;
	event.data.l[2] = 0;
	event.data.l[3] = 0;
	event.data.l[4] = 0;
	XSendEvent(QX11Info::display(), rootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, reinterpret_cast<XEvent*>(&event));
}

void X11Support::minimizeWindow(unsigned long window)
{
	XIconifyWindow(QX11Info::display(), window, QX11Info::appScreen());
}

unsigned long X11Support::systemTrayAtom()
{
	return atom(QString("_NET_SYSTEM_TRAY_S") + QString::number(QX11Info::appScreen()));
}

bool X11Support::makeSystemTray(unsigned long window)
{
	if(XGetSelectionOwner(QX11Info::display(), systemTrayAtom()) != 0)
		return false;

	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), window, CurrentTime);
	return true;
}

void X11Support::freeSystemTray()
{
	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), None, CurrentTime);
}
