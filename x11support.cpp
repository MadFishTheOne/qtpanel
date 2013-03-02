#include "x11support.h"

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include <qpa/qplatformnativeinterface.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

X11Support* X11Support::m_instance = NULL;

X11Support::X11Support()
{
	m_instance = this;

	m_connection = reinterpret_cast<xcb_connection_t*>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("connection", NULL));
	m_rootWindow = static_cast<xcb_window_t>(QApplication::desktop()->windowHandle()->winId());

	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

/*	int damageErrorBase;
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

	if(event->response_type == XCB_PROPERTY_NOTIFY)
	{
		xcb_property_notify_event_t* t = reinterpret_cast<xcb_property_notify_event_t*>(event);
		emit windowPropertyChanged(t->window, t->atom);
	}

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

xcb_atom_t X11Support::atom(const QString& name)
{
	if(!m_instance->m_cachedAtoms.contains(name))
	{
		xcb_atom_t atom = 0;
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection(), 0, name.toLatin1().length(), name.toLatin1().data());
		xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection(), cookie, NULL);
		if(reply != NULL)
		{
			atom = reply->atom;
			free(reply);
		}
		m_instance->m_cachedAtoms[name] = atom;
	}
	return m_instance->m_cachedAtoms[name];
}

void X11Support::removeWindowProperty(xcb_window_t window, const QString& name)
{
	xcb_delete_property(connection(), window, atom(name));
}

void X11Support::setWindowPropertyCardinalArray(xcb_window_t window, const QString& name, const QVector<uint32_t>& values)
{
	xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, window, atom(name), XCB_ATOM_CARDINAL, 32, values.size(), values.data());
}

void X11Support::setWindowPropertyCardinal(xcb_window_t window, const QString& name, uint32_t value)
{
	xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, window, atom(name), XCB_ATOM_CARDINAL, 32, 1, &value);
}

void X11Support::setWindowPropertyAtom(xcb_window_t window, const QString& name, const QString& value)
{
	xcb_atom_t t = atom(value);
	xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, window, atom(name), XCB_ATOM_ATOM, 32, 1, &t);
}

void X11Support::setWindowPropertyVisualId(xcb_window_t window, const QString& name, xcb_visualid_t value)
{
	xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, window, atom(name), XCB_ATOM_VISUALID, 32, 1, &value);
}

template<class T>
static T getWindowPropertyScalar(xcb_window_t window, xcb_atom_t atom, xcb_atom_t type)
{
	T result = 0;
	xcb_get_property_cookie_t cookie = xcb_get_property(X11Support::connection(), 0, window, atom, type, 0, 1);
	xcb_get_property_reply_t* reply = xcb_get_property_reply(X11Support::connection(), cookie, NULL);
	if(reply != NULL)
	{
		if(reply->length > 0)
			result = *reinterpret_cast<const T*>(xcb_get_property_value(reply));
		free(reply);
	}
	return result;
}

template<class T>
static QVector<T> getWindowPropertyArray(xcb_window_t window, xcb_atom_t atom, xcb_atom_t type)
{
	QVector<T> result;
	xcb_get_property_cookie_t cookie = xcb_get_property(X11Support::connection(), 0, window, atom, type, 0, 0xFFFFFFFF);
	xcb_get_property_reply_t* reply = xcb_get_property_reply(X11Support::connection(), cookie, NULL);
	if(reply != NULL)
	{
		const T* values = reinterpret_cast<const T*>(xcb_get_property_value(reply));
		result.resize(reply->length);
		for(int i = 0; i < reply->length; i++)
			result[i] = values[i];
		free(reply);
	}
	return result;
}

uint32_t X11Support::getWindowPropertyCardinal(xcb_window_t window, const QString& name)
{
	return getWindowPropertyScalar<uint32_t>(window, atom(name), XCB_ATOM_CARDINAL);
}

xcb_window_t X11Support::getWindowPropertyWindow(xcb_window_t window, const QString& name)
{
	return getWindowPropertyScalar<xcb_window_t>(window, atom(name), XCB_ATOM_WINDOW);
}

QVector<xcb_window_t> X11Support::getWindowPropertyWindowsArray(xcb_window_t window, const QString& name)
{
	return getWindowPropertyArray<xcb_window_t>(window, atom(name), XCB_ATOM_WINDOW);
}

QVector<xcb_atom_t> X11Support::getWindowPropertyAtomsArray(xcb_window_t window, const QString& name)
{
	return getWindowPropertyArray<xcb_atom_t>(window, atom(name), XCB_ATOM_ATOM);
}

QString X11Support::getWindowPropertyUTF8String(xcb_window_t window, const QString& name)
{
	QString result;
	xcb_get_property_cookie_t cookie = xcb_get_property(connection(), 0, window, atom(name), atom("UTF8_STRING"), 0, 0xFFFFFFFF);
	xcb_get_property_reply_t* reply = xcb_get_property_reply(connection(), cookie, NULL);
	if(reply != NULL)
	{
		result = QString::fromUtf8(reinterpret_cast<const char*>(xcb_get_property_value(reply)));
		free(reply);
	}
	return result;
}

QString X11Support::getWindowPropertyLatin1String(xcb_window_t window, const QString& name)
{
	QString result;
	xcb_get_property_cookie_t cookie = xcb_get_property(connection(), 0, window, atom(name), XCB_ATOM_STRING, 0, 0xFFFFFFFF);
	xcb_get_property_reply_t* reply = xcb_get_property_reply(connection(), cookie, NULL);
	if(reply != NULL)
	{
		result = QString::fromUtf8(reinterpret_cast<const char*>(xcb_get_property_value(reply)));
		free(reply);
	}
	return result;
}

QString X11Support::getWindowName(xcb_window_t window)
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

QIcon X11Support::getWindowIcon(xcb_window_t window)
{
	QIcon icon;

	QVector<uint32_t> array = getWindowPropertyArray<uint32_t>(window, atom("_NET_WM_ICON"), XCB_ATOM_CARDINAL);

	const uint32_t* data = array.data();
	int numItems = array.size();

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

	return icon;
}

bool X11Support::getWindowUrgency(xcb_window_t window)
{
	xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints(connection(), window);
	xcb_icccm_wm_hints_t hints;
	xcb_icccm_get_wm_hints_reply(connection(), cookie, &hints, NULL);
	return xcb_icccm_wm_hints_get_urgency(&hints) != 0;
}

void X11Support::registerForWindowPropertyChanges(xcb_window_t window)
{
	uint32_t t = XCB_EVENT_MASK_PROPERTY_CHANGE;
	xcb_change_window_attributes(connection(), window, XCB_CW_EVENT_MASK, &t);
}

void X11Support::registerForTrayIconUpdates(xcb_window_t window)
{
/*	XSelectInput(QX11Info::display(), window, StructureNotifyMask);

	// Apparently, there is no need to destroy damage object, as it's gone automatically when window is destroyed.
	XDamageCreate(QX11Info::display(), window, XDamageReportNonEmpty);*/
}

static void sendNETWMMessage(xcb_window_t window, const QString& atomName, uint32_t l0 = 0, uint32_t l1 = 0, uint32_t l2 = 0, uint32_t l3 = 0, uint32_t l4 = 0)
{
	char buf[32];
	memset(buf, 0, sizeof(buf));
	xcb_client_message_event_t* event = reinterpret_cast<xcb_client_message_event_t*>(buf);
	event->response_type = XCB_CLIENT_MESSAGE;
	event->window = window;
	event->type = X11Support::atom(atomName);
	event->format = 32;
	event->data.data32[0] = l0;
	event->data.data32[1] = l1;
	event->data.data32[2] = l2;
	event->data.data32[3] = l3;
	event->data.data32[4] = l4;
	xcb_send_event(X11Support::connection(), 0, X11Support::rootWindow(), XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, buf);
}

void X11Support::activateWindow(xcb_window_t window)
{
	uint32_t t = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(connection(), window, XCB_CONFIG_WINDOW_STACK_MODE, &t);

	// Apparently, KWin won't bring window to top with configure request,
	// so we also need to ask it politely by sending a message.
	sendNETWMMessage(window, "_NET_ACTIVE_WINDOW", 2, 0);
}

void X11Support::minimizeWindow(xcb_window_t window)
{
	sendNETWMMessage(window, "WM_CHANGE_STATE", 3);
}

void X11Support::closeWindow(xcb_window_t window)
{
	sendNETWMMessage(window, "_NET_CLOSE_WINDOW", 0, 2);
}

xcb_atom_t X11Support::systemTrayAtom()
{
//	return atom(QString("_NET_SYSTEM_TRAY_S") + QString::number(QX11Info::appScreen()));
	return 0;
}

bool X11Support::makeSystemTray(xcb_window_t window)
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

xcb_visualid_t X11Support::getARGBVisualId()
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

void X11Support::redirectWindow(xcb_window_t window)
{
//	XCompositeRedirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

void X11Support::unredirectWindow(xcb_window_t window)
{
//	XCompositeUnredirectWindow(QX11Info::display(), window, CompositeRedirectManual);
}

QPixmap X11Support::getWindowPixmap(xcb_window_t window)
{
//	return QPixmap::fromX11Pixmap(XCompositeNameWindowPixmap(QX11Info::display(), window));
}

void X11Support::resizeWindow(xcb_window_t window, int width, int height)
{
//	XResizeWindow(QX11Info::display(), window, width, height);
}

void X11Support::moveWindow(xcb_window_t window, int x, int y)
{
//	XMoveWindow(QX11Info::display(), window, x, y);
}

void X11Support::mapWindow(xcb_window_t window)
{
//	XMapWindow(QX11Info::display(), window);
}

void X11Support::reparentWindow(xcb_window_t window, xcb_window_t parent)
{
/*	XReparentWindow(QX11Info::display(), window, parent, 0, 0);
	XSync(QX11Info::display(), False);*/
}

void X11Support::setWindowBackgroundBlack(xcb_window_t window)
{
//	XSetWindowBackground(QX11Info::display(), window, BlackPixel(QX11Info::display(), QX11Info::appScreen()));
}
