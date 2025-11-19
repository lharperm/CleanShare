set(PRESENTATION_SOURCES
        src/MainWindow.cpp
        src/ImagePreviewWidget.cpp
)

set(PRESENTATION_HEADERS
        include/MainWindow.h
        include/ImagePreviewWidget.h
)

add_library(cleanshare_presentation
        ${PRESENTATION_SOURCES}
        ${PRESENTATION_HEADERS}
)

target_include_directories(cleanshare_presentation
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cleanshare_presentation
        PUBLIC Qt6::Widgets
)
