# Core library: SessionController, ImageSession, etc.

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.cc
        src/*.cxx
)

file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS
        include/*.h
        include/*.hpp
)

add_library(cleanshare_core STATIC
        ${CORE_SOURCES}
        ${CORE_HEADERS}
)

target_include_directories(cleanshare_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cleanshare_core
        PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

if(OpenCV_FOUND)
    target_link_libraries(cleanshare_core PUBLIC ${OpenCV_LIBS})
    target_include_directories(cleanshare_core PUBLIC ${OpenCV_INCLUDE_DIRS})
endif()

