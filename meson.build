project(
  'salmagundi',
  'c',
  version : '1.0.0',
)

lib_salmagundi = library(
  'salmagundi',
  'src/salmagundi.c',
  include_directories : ['include'],
  install : true,
)

test_salmagundi = executable(
  'test-salmagundi',
  'tests/test-salmagundi.c',
  include_directories : ['include'],
  install : false,
  link_with : [lib_salmagundi],
)

test('basic', test_salmagundi)
