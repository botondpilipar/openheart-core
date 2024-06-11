
exp_files = files(
    'experiments_main.c',
    'g_lib_exp.c',
    'g_lib_exp.h',
    'kernel_exp.h',
    'kernel_exp.c'
)

executable('openheart-experiments',
           exp_files,
           install : false,
           dependencies : [libcurl_dep, glib_dep, liblogc_dep, libkernel_dep, libjson_dep],
           c_pch : join_paths(pchdir, 'core_pch.h')
)
