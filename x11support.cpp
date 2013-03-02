#include "x11support.h"

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include <qpa/qplatformnativeinterface.h>

#include <xcb/xcb.h>

/*static XErrorHandler oldX11ErrorHandler = NULL;

static int x11errorHandler(Display* display, XErrorEvent* error)
{
	if(error->error_code == BadWindow)
		return 0; // This usually happens when querying property on a window that's already gone. That's OK.

	return (*oldX11ErrorHandler)(display, error);
}*/

X11Support* X11Support::m_instance = NULL;

X11Support::X11Support()
{
	m_instance = this;

	m_connection = reinterpret_cast<xcb_connection_t*>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("connection", NULL));
	m_rootWindow = static_cast<unsigned long>(QApplication::desktop()->windowHandle()->winId());

	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

/*	oldX11ErrorHandler = XSetErrorHandler(x11errorHandler);
	int damageErrorBase;
	XDamageQueryExtension(QX11Info::display(), &m_damageEventBase, &damageErrorBase);*/
}

X11Support::~X11Support()
{
	QAbstractEventDispatcher::instance()->removeNativeEventFilter(this);
	m_instance = NULL;
}

bool X11Support::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
	xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t*>(message);

/*	if(event->type == m_damageEventBase + XDamageNotify)
	{
		// Repair damaged area.
		XDamageNotifyEvent* damageEvent = reinterpret_cast<XDamageNotifyEvent*>(event);
		XDamageSubtract(QX11Info::display(), damageEvent->damage, None, None);

		emit windowDamaged(event->xany.window);
	}
	if(event->type == DestroyNotify)
		emit windowClosed(event->xdestroywindow.window);
	if(event->type == ConfigureNotify)
		emit windowReconfigured(event->xconfigure.window, event->xconfigure.x, event->xconfigure.y, event->xconfigure.width, event->xconfigure.height);
	if(event->type == PropertyNotify)
		emit windowPropertyChanged(event->xproperty.window, event->xproperty.atom);
	if(event->type == ClientMessage)
		emit clientMessageReceived(event->xclient.window, event->xclient.message_type, event->xclient.data.b);*/

	return false;
}

unsigned long X11Support::rootWindow()
{
//	return QX11Info::appRootWindow();
	return 0;
}

unsigned long X11Support::atom(const QString& name)
{
/*	if(!m_instance->m_cachedAtoms.contains(name))
		m_instance->m_cachedAtoms[name] = XInternAtom(QX11Info::display(), name.toLatin1().data(), False);
	return m_instance->m_cachedAtoms[name];*/
	return 0;
}

void X11Support::removeWindowProperty(unsigned long window, const QString& name)
{
//	XDeleteProperty(QX11Info::display(), window, atom(name));
}

void X11Support::setWindowPropertyCardinalArray(unsigned long window, const QString& name, const QVector<unsigned long>& values)
{
//	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(values.data()), values.size());
}

void X11Support::setWindowPropertyCardinal(unsigned long window, const QString& name, unsigned long value)
{
//	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&value), 1);
}

void X11Support::setWindowPropertyVisualId(unsigned long window, const QString& name, unsigned long value)
{
//	XChangeProperty(QX11Info::display(), window, atom(name), XA_VISUALID, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&value), 1);
}

template<class T>
static bool getWindowPropertyHelper(unsigned long window, unsigned long atom, unsigned long type, int& numItems, T*& data)
{
/*	Atom retType;
	int retFormat;
	unsigned long numItemsTemp;
	unsigned long bytesLeft;
	if(XGetWindowProperty(QX11Info::display(), window, atom, 0, 0x7FFFFFFF, False, type, &retType, &retFormat, &numItemsTemp, &bytesLeft, reinterpret_cast<unsigned char**>(&data)) != Success)
		return false;
	numItems = numItemsTemp;
	if(numItems == 0)
		return false;
	return true;*/
	return false;
}

unsigned long X11Support::getWindowPropertyCardinal(unsigned long window, const QString& name)
{
/*	int numItems;
	unsigned long* data;
	unsigned long value = 0;
	if(!getWindowPropertyHelper(window, atom(name), XA_CARDINAL, numItems, data))
		return value;
	value = data[0];
	XFree(data);
	return value;*/
	return 0;
}

unsigned long X11Support::getWindowPropertyWindow(unsigned long window, const QString& name)
{
/*	int numItems;
	unsigned long* data;
	unsigned long value = 0;
	if(!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return value;
	value = data[0];
	XFree(data);
	return value;*/
	return 0;
}

QVector<unsigned long> X11Support::getWindowPropertyWindowsArray(unsigned long window, const QString& name)
{
/*	int numItems;
	unsigned long* data;
	QVector<unsigned long> values;
	if(!getWindowPropertyHelper(window, atom(name), XA_WINDOW, numItems, data))
		return values;
	for(int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;*/
	return QVector<unsigned long>();
}

QVector<unsigned long> X11Support::getWindowPropertyAtomsArray(unsigned long window, const QString& name)
{
/*	int numItems;
	unsigned long* data;
	QVector<unsigned long> values;
	if(!getWindowPropertyHelper(window, atom(name), XA_ATOM, numItems, data))
		return values;
	for(int i = 0; i < numItems; i++)
		values.append(data[i]);
	XFree(data);
	return values;*/
	return QVector<unsigned long>();
}

QString X11Support::getWindowPropertyUTF8String(unsigned long window, const QString& name)
{
/*	int numItems;
	char* data;
	QString value;
	if(!getWindowPropertyHelper(window, atom(name), atom("UTF8_STRING"), numItems, data))
		return value;
	value = QString::fromUtf8(data);
	XFree(data);
	return value;*/
	return "";
}

QString X11Support::getWindowPropertyLatin1String(unsigned long window, const QString& name)
{
/*	int numItems;
	char* data;
	QString value;
	if(!getWindowPropertyHelper(window, atom(name), XA_STRING, numItems, data))
		return value;
	value = QString::fromLatin1(data);
	XFree(data);
	return value;*/
	return "";
}

QString X11Support::getWindowName(unsigned long window)
{
/*	QString result = getWindowPropertyUTF8String(window, "_NET_WM_VISIBLE_NAME");
	if(result.isEmpty())
		result = getWindowPropertyUTF8String(window, "_NET_WM_NAME");
	if(result.isEmpty())
		result = getWindowPropertyLatin1String(window, "WM_NAME");
	if(result.isEmpty())
		result = "<Unknown>";
	return result;*/
	return "";
}

QIcon X11Support::getWindowIcon(unsigned long window)
{
/*	int numItems;
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
	return icon;*/
	return QIcon();
}

bool X11Support::getWindowUrgency(unsigned long window)
{
/*	XWMHints* hints = XGetWMHints(QX11Info::display(), window);
	if(hints == NULL)
		return false;
	bool isUrgent = (hints->flags & 256) != 0; // UrgencyHint
	XFree(hints);
	return isUrgent;*/
	return false;
}

void X11Support::registerForWindowPropertyChanges(unsigned long window)
{
//	XSelectInput(QX11Info::display(), window, PropertyChangeMask);
}

void X11Support::registerForTrayIconUpdates(unsigned long window)
{
/*	XSelectInput(QX11Info::display(), window, StructureNotifyMask);

	// Apparently, there is no need to destroy damage object, as it's gone automatically when window is destroyed.
	XDamageCreate(QX11Info::display(), window, XDamageReportNonEmpty);*/
}

static void sendNETWMMessage(unsigned long window, const QString& atomName, unsigned long l0 = 0, unsigned long l1 = 0, unsigned long l2 = 0, unsigned long l3 = 0, unsigned long l4 = 0)
{
/*	XClientMessageEvent event;
	event.type = ClientMessage;
	event.window = window;
	event.message_type = X11Support::atom(atomName);
	event.format = 32;
	event.data.l[0] = l0;
	event.data.l[1] = l1;
	event.data.l[2] = l2;
	event.data.l[3] = l3;
	event.data.l[4] = l4;
	XSendEvent(QX11Info::display(), X11Support::rootWindow(), False, SubstructureNotifyMask | SubstructureRedirectMask, reinterpret_cast<XEvent*>(&event));*/
}

void X11Support::activateWindow(unsigned long window)
{
/*	XWindowChanges wc;
	wc.stack_mode = Above;
	XConfigureWindow(QX11Info::display(), window, CWStackMode, &wc);

	// Apparently, KWin won't bring window to top with configure request,
	// so we also need to ask it politely by sending a message.
	sendNETWMMessage(window, "_NET_ACTIVE_WINDOW", 2, CurrentTime);*/
}

void X11Support::minimizeWindow(unsigned long window)
{
//	XIconifyWindow(QX11Info::display(), window, QX11Info::appScreen());
}

void X11Support::closeWindow(unsigned long window)
{
//	sendNETWMMessage(window, "_NET_CLOSE_WINDOW", CurrentTime, 2);
}

void X11Support::destroyWindow(unsigned long window)
{
//	XDestroyWindow(QX11Info::display(), window);
}

void X11Support::killClient(unsigned long window)
{
//	XKillClient(QX11Info::display(), window);
}

unsigned long X11Support::systemTrayAtom()
{
//	return atom(QString("_NET_SYSTEM_TRAY_S") + QString::number(QX11Info::appScreen()));
	return 0;
}

bool X11Support::makeSystemTray(unsigned long window)
{
/*	if(XGetSelectionOwner(QX11Info::display(), systemTrayAtom()) != 0)
		return false;

	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), window, CurrentTime);
	setWindowPropertyVisualId(window, "_NET_SYSTEM_TRAY_VISUAL", getARGBVisualId());
	XSync(QX11Info::display(), False);

	// Inform other clients.
	XClientMessageEvent event;
	event.type = ClientMessage;
	event.window = rootWindow();
	event.message_type = atom("MANAGER");
	event.format = 32;
	event.data.l[0] = CurrentTime;
	event.data.l[1] = systemTrayAtom();
	event.data.l[2] = window;
	event.data.l[3] = 0;
	event.data.l[4] = 0;
	XSendEvent(QX11Info::display(), X11Support::rootWindow(), False, StructureNotifyMask, reinterpret_cast<XEvent*>(&event));*/

	return true;
}

void X11Support::freeSystemTray()
{
//	XSetSelectionOwner(QX11Info::display(), systemTrayAtom(), None, CurrentTime);
}

unsigned long X11Support::getARGBVisualId()
{
/*	XVisualInfo visualInfoTemplate;
	visualInfoTemplate.screen = QX11Info::appScreen();
	visualInfoTemplate.depth = 32;
	visualInfoTemplate.red_mask = 0x00FF0000;
	visualInfoTemplate.green_mask = 0x0000FF00;
	visualInfoTemplate.blue_mask = 0x000000FF;

	int numVisuals;
	XVisualInfo* visualInfoList = XGetVisualInfo(QX11Info::display(), VisualScreenMask | VisualDepthMask | VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask, &visualInfoTemplate, &numVisuals);
	unsigned long id = visualInfoList[0].visualid;
	XFree(visualInfoList);

	return id;*/
	return 0;
}

void X11Support::redirectWindow(unsigned long window)
{
//	XCompositeRedirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

void X11Support::unredirectWindow(unsigned long window)
{
//	XCompositeUnredirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

QPixmap X11Support::getWindowPixmap(unsigned long window)
{
//	return QPixmap::fromX11Pixmap(XCompositeNameWindowPixmap(QX11Info::display(), window));
}

void X11Support::resizeWindow(unsigned long window, int width, int height)
{
//	XResizeWindow(QX11Info::display(), window, width, height);
}

void X11Support::moveWindow(unsigned long window, int x, int y)
{
//	XMoveWindow(QX11Info::display(), window, x, y);
}

void X11Support::mapWindow(unsigned long window)
{
//	XMapWindow(QX11Info::display(), window);
}

void X11Support::reparentWindow(unsigned long window, unsigned long parent)
{
/*	XReparentWindow(QX11Info::display(), window, parent, 0, 0);
	XSync(QX11Info::display(), False);*/
}

void X11Support::setWindowBackgroundBlack(unsigned long window)
{
//	XSetWindowBackground(QX11Info::display(), window, BlackPixel(QX11Info::display(), QX11Info::appScreen()));
}
