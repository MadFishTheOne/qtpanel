#ifndef PANELAPPLICATION_H
#define PANELAPPLICATION_H

#include <QtGui/QApplication>
#include "panelwindow.h"

class PanelApplication: public QApplication
{
	Q_OBJECT
public:
	PanelApplication(int& argc, char** argv);
	~PanelApplication();

private:
	PanelWindow* m_panelWindow;
};

#endif
