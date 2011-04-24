#include "panelapplication.h"

#include <QtGui/QX11Info>
#include "x11support.h"

// Keep all the X11 stuff with scary defines below normal headers.
#include <X11/Xlib.h>

static XErrorHandler oldX11ErrorHandler = NULL;

static int x11errorHandler(Display* display, XErrorEvent* error)
{
	if(error->error_code == BadWindow)
		return 0; // This usually happens when querying property on a window that's already gone. That's OK.

	return (*oldX11ErrorHandler)(display, error);
}

PanelApplication* PanelApplication::m_instance = NULL;

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_instance = this;

	oldX11ErrorHandler = XSetErrorHandler(x11errorHandler);

	// TODO: Make this configurable.
	QIcon::setThemeName("Faenza-Dark");

	m_x11support = new X11Support();

	m_panelWindow = new PanelWindow();
	m_panelWindow->resize(128, 32);
	m_panelWindow->setLayoutPolicy(PanelWindow::FillSpace);
	//m_panelWindow->setVerticalAnchor(PanelWindow::Max);
	m_panelWindow->setDockMode(true);
	m_panelWindow->init();
	m_panelWindow->show();
}

PanelApplication::~PanelApplication()
{
	delete m_panelWindow;
	delete m_x11support;

	m_instance = NULL;
}

bool PanelApplication::x11EventFilter(XEvent* event)
{
	if(event->type == PropertyNotify)
		emit windowPropertyChanged(event->xproperty.window, event->xproperty.atom);
	if(event->type == ClientMessage)
		emit clientMessageReceived(event->xclient.window, event->xclient.message_type, event->xclient.data.b);
	return false;
}
