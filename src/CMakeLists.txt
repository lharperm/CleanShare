# Sub-libraries for modules
add_subdirectory(core)
add_subdirectory(presentation)
# Later you can add: add_subdirectory(detection) add_subdirectory(redaction) etc.

set(APP_SOURCES
        main.cpp
)

# Only use the resources that actually exist
set(APP_RESOURCES
        ../assets/icons/Logo.qrc
        ${CMAKE_CURRENT_LIST_DIR}/../appicon.rc
)

qt_add_executable(cleanshare_app
        MANUAL_FINALIZATION
        ${APP_SOURCES}
        ${APP_RESOURCES}
)

target_link_libraries(cleanshare_app
        PRIVATE
        cleanshare_core
        cleanshare_presentation
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)


target_include_directories(cleanshare_app
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/core/include
        ${CMAKE_CURRENT_LIST_DIR}/presentation/include
)

set_target_properties(cleanshare_app PROPERTIES
        OUTPUT_NAME "CleanShare"
)

qt_finalize_executable(cleanshare_app)
