#include "panelapplication.h"

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	m_panelWindow = new PanelWindow();
	m_panelWindow->setScreen(0);
	m_panelWindow->show();
}

PanelApplication::~PanelApplication()
{
	delete m_panelWindow;
}
