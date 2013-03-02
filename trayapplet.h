#ifndef TRAYAPPLET_H
#define TRAYAPPLET_H

#include <stdint.h>
#include <QtCore/QVector>
#include "applet.h"

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;

class TrayApplet: public Applet
{
	Q_OBJECT
public:
	TrayApplet(PanelWindow* panelWindow);
	~TrayApplet();

	bool init();

	QSize desiredSize();

protected:
	void layoutChanged();

private slots:
	void clientMessageReceived(xcb_window_t window, xcb_atom_t atom, const uint32_t* data);
	void windowClosed(xcb_window_t window);
	void windowReconfigured(xcb_window_t window, int x, int y, int width, int height);

private:
	void updateLayout();

	bool m_initialized;
	QVector<xcb_window_t> m_trayItems;
	int m_iconSize;
	int m_spacing;
};

#endif
