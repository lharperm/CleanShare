# Presentation / GUI library: MainWindow, widgets, etc.

file(GLOB_RECURSE PRESENTATION_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.cc
        src/*.cxx
)

file(GLOB_RECURSE PRESENTATION_HEADERS CONFIGURE_DEPENDS
        include/*.h
        include/*.hpp
)

add_library(cleanshare_presentation STATIC
        ${PRESENTATION_SOURCES}
        ${PRESENTATION_HEADERS}
)

target_include_directories(cleanshare_presentation
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

target_link_libraries(cleanshare_presentation
        PUBLIC
        cleanshare_core
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)
