#include "panelapplication.h"

#include "x11support.h"

PanelApplication* PanelApplication::m_instance = NULL;

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_instance = this;

	m_x11support = new X11Support();

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
	m_x11support->onX11Event(event);
	return false;
}
