#include "spacerapplet.h"

SpacerApplet::SpacerApplet(PanelWindow* panelWindow)
	: Applet(panelWindow)
{

}

SpacerApplet::~SpacerApplet()
{

}

QSize SpacerApplet::desiredSize()
{
	return QSize(-1, -1);
}
