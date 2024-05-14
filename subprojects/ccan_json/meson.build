project('ccan_json', 'c')

ccan_json_incdir = include_directories('.')

libjson_headers = files(
    'json.h'
)

sources = files(
    'json.c'
)

libjson = static_library(
  'ccan_json',
  sources,
  include_directories : ccan_json_incdir,
  install : true)

libjson_dep = declare_dependency(include_directories : ccan_json_incdir, link_with : libjson)
