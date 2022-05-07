zathura-cb
==========

The zathura-cb plugin adds comic book support to zathura.

Requirements
------------

* `zathura` (>= 0.2.0)
* `libarchive`
* `girara`
* `cairo`

Installation
------------

To build and install the plugin using meson's ninja backend, use:

  meson build
  cd build
  ninja
  ninja install

Note that the default backend for meson might vary based on the platform. Please
refer to the meson documentation for platform specific dependencies.

Bugs
----

Please report bugs at http://git.pwmt.org/pwmt/zathura-cb.
