libbadcpu_src = files(
  'badcpu.cpp',
  'cpuid.cpp',
  'decoder.cpp',
  'emulator.cpp',
  'signal_handler.cpp',
)

libbadcpu = shared_library('badcpu',
  libbadcpu_src,
  include_directories: include_directories('../../include'),
  dependencies: dependency('threads'),
  install: true,
)

libbadcpu_dep = declare_dependency(
  link_with: libbadcpu,
  include_directories: include_directories('../../include'),
)

test_badcpu = executable('test_badcpu',
  'test/test_badcpu.cpp',
  include_directories: include_directories('../../include'),
  link_with: libbadcpu,
)

test('badcpu-test', test_badcpu)