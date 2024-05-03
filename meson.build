project('openheart-core', 'c',
  version : '0.1',
  default_options : ['warning_level=3'],
  license: 'GPL-2.0-or-later',
  meson_version: '>= 1.4.0'
)

app_id = 'org.openheart.Core'

cc = meson.get_compiler('c')
pchdir = join_paths(meson.project_source_root(), 'pch')

libcurl_dep = dependency('libcurl', version : '>=8.1.0')
glib_dep = dependency('glib-2.0', version : '>=2.79')
logc_proj = subproject('logc')
liblogc_dep = logc_proj.get_variable('liblogc_dep')

subdir('lib')
subdir('qobject')
subdir('gobject')
subdir('test')

executable('openheart-core',
           'main.c',
           install : false,
           dependencies : [libcurl_dep, glib_dep, liblogc_dep],
           include_directories : lib_incdir,
           c_pch : join_paths(pchdir, 'core_pch.h')
)
