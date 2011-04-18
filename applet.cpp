#include "applet.h"

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
}

Applet::~Applet()
{
}

void Applet::setSize(const QSize& size)
{
	m_size = size;
	update();
}
