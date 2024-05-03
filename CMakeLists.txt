CMAKE_MINIMUM_REQUIRED(VERSION 3.28)
project("openheart-core"
    VERSION "1.0.0"
    DESCRIPTION "Core library for OpenHeart graphical kernel application"
    LANGUAGES C CXX
)

include(CMakePrintHelpers)


find_package(CURL REQUIRED)
find_package(glib REQUIRED)
find_package(log.c REQUIRED)


add_subdirectory(lib)
add_subdirectory(gobject)
add_subdirectory(qobject)
add_subdirectory(test)


add_executable(${PROJECT_NAME}
    main.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE lib/inc
    ${GLIB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIR}
    ${log.c_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${GLIB_LDFLAGS}
    CURL::libcurl
    log.c::log.c
)
