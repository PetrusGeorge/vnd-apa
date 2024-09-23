project(
  'apa',
  'cpp',
  default_options: [
    'cpp_std=c++23',
    'buildtype=release',
    'warning_level=3',
    'b_ndebug=if-release',
    'b_lto=true',
  ],
)

subdir('src')
argparse_dep = dependency('argparse')

extra_args = ['-march=native']

executable('apa', sources, cpp_args: extra_args, dependencies: argparse_dep, install: true)
