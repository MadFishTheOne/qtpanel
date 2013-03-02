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
	const xcb_setup_t* setup = xcb_get_setup(m_connection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	m_screenNumber = 0;

	for(;;)
	{
		if(iter.data->root == m_rootWindow)
		{
			m_screen = iter.data;
			break;
		}
		m_screenNumber++;
		xcb_screen_next(&iter);
	}

	m_systemTrayAtom = atom(QString("_NET_SYSTEM_TRAY_S") + QString::number(m_screenNumber));

	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}

X11Support::~X11Support()
{
	QAbstractEventDispatcher::instance()->removeNativeEventFilter(this);
	m_instance = NULL;
}

bool X11Support::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
	xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t*>(message);

	switch(event->response_type & ~0x80)
	{
		case XCB_PROPERTY_NOTIFY:
		{
			xcb_property_notify_event_t* t = reinterpret_cast<xcb_property_notify_event_t*>(event);
			emit windowPropertyChanged(t->window, t->atom);
			break;
		}
		case XCB_DESTROY_NOTIFY:
		{
			xcb_destroy_notify_event_t* t = reinterpret_cast<xcb_destroy_notify_event_t*>(event);
			emit windowClosed(t->window);
			break;
		}
		case XCB_CONFIGURE_NOTIFY:
		{
			xcb_configure_notify_event_t* t = reinterpret_cast<xcb_configure_notify_event_t*>(event);
			emit windowReconfigured(t->window, t->x, t->y, t->width, t->height);
			break;
		}
		case XCB_CLIENT_MESSAGE:
		{
			xcb_client_message_event_t* t = reinterpret_cast<xcb_client_message_event_t*>(event);
			emit clientMessageReceived(t->window, t->type, t->data.data32);
		}
		default:
		{
			break;
		}
	}

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
		result = QString::fromUtf8(reinterpret_cast<const char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply));
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
		result = QString::fromLatin1(reinterpret_cast<const char*>(xcb_get_property_value(reply)), xcb_get_property_value_length(reply));
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

bool X11Support::registerForTrayIconUpdates(xcb_window_t window)
{
	uint32_t t = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
	xcb_void_cookie_t cookie = xcb_change_window_attributes(connection(), window, XCB_CW_EVENT_MASK, &t);
	xcb_generic_error_t* error = xcb_request_check(connection(), cookie);
	if(error != NULL)
	{
		free(error);
		return false;
	}

	return true;
}

static void sendClientMessage(xcb_window_t window, xcb_atom_t type, uint32_t eventMask, uint32_t l0 = 0, uint32_t l1 = 0, uint32_t l2 = 0, uint32_t l3 = 0, uint32_t l4 = 0)
{
	char buf[32];
	memset(buf, 0, sizeof(buf));
	xcb_client_message_event_t* event = reinterpret_cast<xcb_client_message_event_t*>(buf);
	event->response_type = XCB_CLIENT_MESSAGE;
	event->window = window;
	event->type = type;
	event->format = 32;
	event->data.data32[0] = l0;
	event->data.data32[1] = l1;
	event->data.data32[2] = l2;
	event->data.data32[3] = l3;
	event->data.data32[4] = l4;
	xcb_send_event(X11Support::connection(), 0, X11Support::rootWindow(), eventMask, buf);
}

static void sendNETWMMessage(xcb_window_t window, xcb_atom_t type, uint32_t l0 = 0, uint32_t l1 = 0, uint32_t l2 = 0, uint32_t l3 = 0, uint32_t l4 = 0)
{
	sendClientMessage(window, type, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, l0, l1, l2, l3, l4);
}

void X11Support::activateWindow(xcb_window_t window)
{
	uint32_t t = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(connection(), window, XCB_CONFIG_WINDOW_STACK_MODE, &t);

	// Apparently, KWin won't bring window to top with configure request,
	// so we also need to ask it politely by sending a message.
	sendNETWMMessage(window, atom("_NET_ACTIVE_WINDOW"), 2, 0);
}

void X11Support::minimizeWindow(xcb_window_t window)
{
	sendNETWMMessage(window, atom("WM_CHANGE_STATE"), 3);
}

void X11Support::closeWindow(xcb_window_t window)
{
	sendNETWMMessage(window, atom("_NET_CLOSE_WINDOW"), 0, 2);
}

bool X11Support::makeSystemTray(xcb_window_t window)
{
	bool alreadyOwned = false;

	xcb_get_selection_owner_cookie_t cookie = xcb_get_selection_owner(connection(), systemTrayAtom());
	xcb_get_selection_owner_reply_t* reply = xcb_get_selection_owner_reply(connection(), cookie, NULL);
	if(reply != NULL)
	{
		alreadyOwned = reply->owner != 0;
		free(reply);
	}

	if(alreadyOwned)
		return false;

	xcb_set_selection_owner(connection(), window, systemTrayAtom(), 0);

	// Inform other clients.
	sendClientMessage(rootWindow(), atom("MANAGER"), XCB_EVENT_MASK_STRUCTURE_NOTIFY, 0, systemTrayAtom(), window);

	return true;
}

void X11Support::freeSystemTray()
{
	xcb_set_selection_owner(connection(), 0, systemTrayAtom(), 0);
}

void X11Support::resizeWindow(xcb_window_t window, int width, int height)
{
	uint32_t values[2] = { width, height };
	xcb_configure_window(connection(), window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

void X11Support::moveWindow(xcb_window_t window, int x, int y)
{
	uint32_t values[2] = { x, y };
	xcb_configure_window(connection(), window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
}

void X11Support::mapWindow(xcb_window_t window)
{
	xcb_map_window(connection(), window);
}

void X11Support::reparentWindow(xcb_window_t window, xcb_window_t parent)
{
	xcb_reparent_window(connection(), window, parent, 0, 0);
}

void X11Support::setWindowBackgroundBlack(xcb_window_t window)
{
	uint32_t t = m_instance->m_screen->black_pixel;
	xcb_change_window_attributes(connection(), window, XCB_CW_BACK_PIXEL, &t);
}
