project('log.c', 'c', 'cpp')

logc_incdir = include_directories('src')

liblogc_headers = files(
    'src/log.h'
)

sources = files(
    'src/log.c'
)

liblogc = static_library(
  'logc',
  sources,
  include_directories : logc_incdir,
  install : true)

liblogc_dep = declare_dependency(include_directories : logc_incdir, link_with : liblogc)
