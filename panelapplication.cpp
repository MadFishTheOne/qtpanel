#include "panelapplication.h"

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_panelWindow = new PanelWindow();
	m_panelWindow->setLocation(0, PanelWindow::Top);
	m_panelWindow->show();
}

PanelApplication::~PanelApplication()
{
	delete m_panelWindow;
}
