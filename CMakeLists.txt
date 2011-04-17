cmake_minimum_required(VERSION 2.8)
project(qtpanel)
find_package(Qt4 REQUIRED)
set(qtpanel_MOC_HEADERS
	panelapplication.h
	panelwindow.h
)
set(qtpanel_SOURCES
	main.cpp
	panelapplication.cpp
	panelwindow.cpp
)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
qt4_wrap_cpp(qtpanel_MOC_OUTPUT ${qtpanel_MOC_HEADERS})
add_executable(qtpanel ${qtpanel_SOURCES} ${qtpanel_MOC_OUTPUT})
target_link_libraries(qtpanel ${QT_LIBRARIES})
