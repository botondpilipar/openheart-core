openheart_main_inc = include_directories('.')

openheart_main = static_library(
    'openheart-core-main',
    'main_process.c',
    dependencies:  [glib_dep, libkernel_dep],
    install: false,
)

openheart_main_dep = declare_dependency(
    include_directories: openheart_main_inc,
    link_with : openheart_main
)
