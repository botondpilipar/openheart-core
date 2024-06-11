project('openheart-core', 'c',
  version : '0.1',
  default_options : ['warning_level=3'],
  license: 'GPL-2.0-or-later',
  meson_version: '>= 1.4.0'
)

app_id = 'org.openheart.Core'

cc = meson.get_compiler('c')
pchdir = join_paths(meson.project_source_root(), 'pch')

logc_proj = subproject('logc')
json_proj = subproject('ccan_json')

libcurl_dep = dependency('libcurl', version : '>=8.1.0')
glib_dep = dependency('glib-2.0', version : '>=2.79')
liblogc_dep = logc_proj.get_variable('liblogc_dep')
libjson_dep = json_proj.get_variable('libjson_dep')

subdir('lib')
subdir('process')
subdir('qobject')
subdir('gobject')
subdir('test')
subdir('experiment')

executable('openheart-core',
           'main.c',
           install : false,
           dependencies : [openheart_main_dep, libcurl_dep, glib_dep, liblogc_dep, libkernel_dep, libjson_dep],
           c_pch : join_paths(pchdir, 'core_pch.h')
)

build_options = meson.build_options()
devenv = environment()
devenv.set('PLUGINS_PATH', meson.current_build_dir())

if build_options.contains('debug')
  devenv.set('G_MESSAGES_DEBUG', 'all')
endif

meson.add_devenv(devenv)
