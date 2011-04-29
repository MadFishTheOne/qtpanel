#include "panelapplication.h"

#include "x11support.h"

// Xlib is needed here for XEvent declaration (used in x11EventFilter).
// Keep all the X11 stuff with scary defines below normal headers.
#include <X11/Xlib.h>

PanelApplication* PanelApplication::m_instance = NULL;

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_instance = this;

	m_x11support = new X11Support();

	// Close existing qtpanel instance.
	QVector<unsigned long> windows = X11Support::getWindowPropertyWindowsArray(X11Support::rootWindow(), "_NET_CLIENT_LIST");
	foreach(unsigned long window, windows)
	{
		if(X11Support::getWindowName(window) == "qtpanel")
		{
			// Have to kill it, since WM may not respect close request on panel window.
			X11Support::killClient(window);
			break;
		}
	}

	// TODO: Make this configurable.
	QIcon::setThemeName("Faenza-Dark");

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
