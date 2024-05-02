CMAKE_MINIMUM_REQUIRED(VERSION 3.28)
project("openheart-core"
    VERSION "1.0.0"
    DESCRIPTION "Core library for OpenHeart graphical kernel application"
    LANGUAGES C CXX
)

find_package(CURL REQUIRED)
find_package (glib REQUIRED)
find_package(log.c REQUIRED)


add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME}
    CURL::libcurl
    glib::glib
    log.c::log.c
)