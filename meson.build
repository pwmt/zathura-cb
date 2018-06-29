project('zathura-cb', 'c',
  version: '0.1.8',
  meson_version: '>=0.43',
  default_options: 'c_std=c11'
)

version = meson.project_version()
version_array = version.split('.')

cc = meson.get_compiler('c')

datadir = get_option('datadir')
desktopdir = join_paths(datadir, 'applications')
metainfodir = join_paths(datadir, 'metainfo')

# required dependencies
zathura = dependency('zathura', version: '>=0.3.8')
girara = dependency('girara-gtk3')
glib = dependency('glib-2.0')
cairo = dependency('cairo')
libarchive = dependency('libarchive')

build_dependencies = [zathura, girara, glib, cairo, libarchive]

if get_option('plugindir') == ''
  plugindir = zathura.get_pkgconfig_variable('plugindir')
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
  '-Wall',
  '-Wextra',
  '-pedantic',
  '-Werror=implicit-function-declaration',
  '-Werror=vla',
  '-fvisibility=hidden'
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
  install_dir: plugindir
)

subdir('data')
