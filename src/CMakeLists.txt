add_subdirectory(presentation)
# add_subdirectory(redaction)   # later
# add_subdirectory(detection)   # later

add_library(cleanshare_core
        core/SessionController.cpp
        core/SessionController.h
)

target_include_directories(cleanshare_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(cleanshare_core
        PUBLIC Qt6::Widgets
)

add_executable(cleanshare_app
        main.cpp
)

target_link_libraries(cleanshare_app
        PRIVATE
        cleanshare_presentation
        cleanshare_core
        Qt6::Widgets
)
