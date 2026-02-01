project('zathura-cb', 'c',
  version: '2026.01.30',
  meson_version: '>=0.61',
  default_options: ['c_std=c17', 'warning_level=3']
)

version = meson.project_version()
version_array = version.split('.')

cc = meson.get_compiler('c')

datadir = get_option('datadir')
desktopdir = join_paths(datadir, 'applications')
metainfodir = join_paths(datadir, 'metainfo')

# required dependencies
zathura = dependency('zathura', version: '>=2026.01.30')
girara = dependency('girara')
glib = dependency('glib-2.0')
cairo = dependency('cairo')
libarchive = dependency('libarchive')

build_dependencies = [zathura, girara, glib, cairo, libarchive]

if get_option('plugindir') == ''
  plugindir = zathura.get_variable(pkgconfig: 'plugindir')
else
  plugindir = get_option('plugindir')
endif

# defines
defines = [
  '-DVERSION_MAJOR=@0@'.format(version_array[0]),
  '-DVERSION_MINOR=@0@'.format(version_array[1]),
  '-DVERSION_REV=@0@'.format(version_array[2]),
  '-D_DEFAULT_SOURCE',
]

# compile flags
flags = [
  '-Werror=implicit-function-declaration',
  '-Werror=vla',
  '-Werror=int-conversion',
  '-Werror=maybe-uninitialized'
]
flags = cc.get_supported_arguments(flags)

sources = files(
  'zathura-cb/document.c',
  'zathura-cb/index.c',
  'zathura-cb/page.c',
  'zathura-cb/plugin.c',
  'zathura-cb/render.c',
  'zathura-cb/utils.c'
)

cb = shared_module('cb',
  sources,
  dependencies: build_dependencies,
  c_args: defines + flags,
  install: true,
  install_dir: plugindir,
  gnu_symbol_visibility: 'hidden'
)

subdir('data')
