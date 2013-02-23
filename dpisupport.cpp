#include "dpisupport.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

int adjustHardcodedPixelSize(int size)
{
    static int dpi = qApp->desktop()->logicalDpiX();
    return size*dpi/96;
}
