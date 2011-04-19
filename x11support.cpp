#include "x11support.h"

#include <QtGui/QApplication>
#include <QtGui/QX11Info>

// Keep all the X11 stuff with scary defines below normal headers.
#include <X11/Xlib.h>
#include <X11/Xatom.h>

X11Support* X11Support::m_instance = NULL;

X11Support::X11Support()
{
	m_instance = this;
}

X11Support::~X11Support()
{
	delete m_instance;
}

unsigned long X11Support::rootWindow()
{
	return QX11Info::appRootWindow();
}

unsigned long X11Support::atom(const QString& name)
{
	if(!m_cachedAtoms.contains(name))
		m_cachedAtoms[name] = XInternAtom(QX11Info::display(), name.toLatin1().data(), False);
	return m_cachedAtoms[name];
}

void X11Support::removeWindowProperty(unsigned long window, const QString& name)
{
	XDeleteProperty(QX11Info::display(), window, atom(name));
}

void X11Support::setWindowPropertyCardinalArray(unsigned long window, const QString& name, const QVector<unsigned long>& values)
{
	XChangeProperty(QX11Info::display(), window, atom(name), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(values.data()), values.size());
}
