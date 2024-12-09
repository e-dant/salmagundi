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
  ['tests/test-salmagundi.c'],
  include_directories : ['include'],
  link_with : lib_salmagundi,
  c_args : ['-DHM_DEBUG'],
  install : false,
)

fuzz_salmagundi = executable(
  'fuzz-salmagundi',
  ['tests/fuzz-salmagundi.c'],
  include_directories : ['include'],
  link_with : lib_salmagundi,
  c_args : ['-DHM_DEBUG', '-fsanitize=fuzzer,address', '-g'],
  link_args : ['-fsanitize=fuzzer,address'],
  install : false,
)

test('test-salmagundi', test_salmagundi)
test('fuzz-salmagundi', fuzz_salmagundi)
