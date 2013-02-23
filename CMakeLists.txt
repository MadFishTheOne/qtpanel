cmake_minimum_required(VERSION 2.8)
project(qtpanel)
find_package(X11 REQUIRED)
find_package(Qt4 REQUIRED)
set(qtpanel_UIS
	panelapplicationsettings.ui
)
set(qtpanel_HEADERS
	dpisupport.h
	iconloader.h
	textgraphicsitem.h
	animationutils.h
)
set(qtpanel_MOC_HEADERS
	x11support.h
	desktopapplications.h
	panelapplication.h
	panelwindow.h
	applet.h
	demoapplet.h
	spacerapplet.h
	clockapplet.h
	applicationsmenuapplet.h
	dockapplet.h
	trayapplet.h
)
set(qtpanel_SOURCES
	dpisupport.cpp
	iconloader.cpp
	x11support.cpp
	desktopapplications.cpp
	main.cpp
	panelapplication.cpp
	panelwindow.cpp
	textgraphicsitem.cpp
	applet.cpp
	demoapplet.cpp
	spacerapplet.cpp
	clockapplet.cpp
	applicationsmenuapplet.cpp
	dockapplet.cpp
	trayapplet.cpp
)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
qt4_wrap_ui(qtpanel_UI_HEADERS ${qtpanel_UIS})
qt4_wrap_cpp(qtpanel_MOC_OUTPUT ${qtpanel_MOC_HEADERS})
include_directories(${CMAKE_BINARY_DIR})
add_executable(qtpanel ${qtpanel_SOURCES} ${qtpanel_MOC_OUTPUT} ${qtpanel_HEADERS} ${qtpanel_UI_HEADERS})
target_link_libraries(qtpanel ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xcomposite_LIB} ${X11_Xdamage_LIB})
