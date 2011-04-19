#include "applet.h"

#include "panelwindow.h"

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
}

Applet::~Applet()
{
}

bool Applet::init()
{
	m_panelWindow->updateLayout();
	return true;
}

void Applet::setRect(const QRect& rect)
{
	m_rect = rect;
	update();
}

void Applet::update()
{
}
