
lib_incdir = include_directories('inc')

libkernel_headers = files(
    'inc/kernel_attributes.h',
    'inc/kernel_data.h',
    'inc/fs_notify.h',
    'inc/array_alg.h',
    'inc/func.h',
    'inc/fs_types.h',
    'inc/fs_notify_handler.h',
    'inc/fs_notify.h',
    'inc/permissions.h',
    'inc/signal_handlers.h',
)

sources = files(
    'src/kernel_attributes.c',
    'src/kernel_data.c',
    'src/fs_notify.c',
    'src/func.c',
    'src/array_alg.c',
    'src/fs_notify.c',
    'src/fs_notify_handler.c',
    'src/permissions.c',
    'src/signal_handlers.c',
)

cap_dep = dependency('libcap')

libkernel = shared_library(
    'libkernel',
    sources,
    dependencies:  [glib_dep, cap_dep],
    include_directories: lib_incdir,
    extra_files: libkernel_headers,
    install : true
)

libkernel_dep = declare_dependency(
    include_directories: lib_incdir,
    link_with : libkernel
)
