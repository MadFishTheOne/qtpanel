#include "dpisupport.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

int adjustHardcodedPixelSize(int size)
{
    static int dpi = qApp->desktop()->logicalDpiX();
    return size*dpi/96;
}
