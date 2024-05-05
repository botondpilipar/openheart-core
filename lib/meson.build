
lib_incdir = include_directories('inc')

libkenel_headers = files(
    'inc/kernel_attributes.h'
)

sources = files(
    'src/kernel_attributes.c',
)

libkernel = shared_library(
    'libkernel',
    sources,
    dependencies:  [glib_dep],
    include_directories: lib_incdir,
    install : true
)

libkernel_dep = declare_dependency(
    include_directories: lib_incdir,
    link_with : libkernel
)
