cmake_minimum_required(VERSION 3.16)

project(CleanShare VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appCleanShare
    main.cpp
    MainWindow.h
    MainWindow.cpp
    SessionController.h
    SessionController.cpp
    ImageCanvas.h
    ImageCanvas.cpp
    Logo.qrc
    appicon.rc
)

if (WIN32)
    target_sources(appCleanShare PRIVATE appicon.rc)
endif()


set_target_properties(appCleanShare PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCleanShare
    PRIVATE Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS appCleanShare
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
