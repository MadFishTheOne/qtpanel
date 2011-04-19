#include "panelapplication.h"

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_panelWindow = new PanelWindow();
	m_panelWindow->resize(128, 24);
	m_panelWindow->setLayoutPolicy(PanelWindow::FillSpace);
	m_panelWindow->setDockMode(true);
	m_panelWindow->init();
	m_panelWindow->show();
}

PanelApplication::~PanelApplication()
{
	delete m_panelWindow;
}
