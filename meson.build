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
  ['tests/test-salmagundi.c', 'src/salmagundi.c'],
  include_directories : ['include'],
  install : false,
  c_args : ['-DHM_DEBUG'],
)

test('basic', test_salmagundi)
