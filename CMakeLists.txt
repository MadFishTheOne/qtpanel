cmake_minimum_required(VERSION 2.8.9)
project(qtpanel)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(XCB REQUIRED)
find_package(Qt5Widgets REQUIRED)
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
qt5_wrap_ui(qtpanel_UI_HEADERS ${qtpanel_UIS})
qt5_wrap_cpp(qtpanel_MOC_OUTPUT ${qtpanel_MOC_HEADERS})
add_executable(qtpanel ${qtpanel_SOURCES} ${qtpanel_MOC_OUTPUT} ${qtpanel_HEADERS} ${qtpanel_UI_HEADERS})
qt5_use_modules(qtpanel Widgets)
target_link_libraries(qtpanel ${XCB_XCB_LIBRARIES} ${XCB_ICCCM_LIBRARIES})

# For <qpa/qplatformnativeinterface.h>.
set_property(TARGET qtpanel APPEND PROPERTY INCLUDE_DIRECTORIES ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
