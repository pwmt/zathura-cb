# See LICENSE file for license and copyright information

VERSION_MAJOR = 0
VERSION_MINOR = 1
VERSION_REV = 4
VERSION = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}

# minimum required zathura version
ZATHURA_MIN_VERSION = 0.2.0
ZATHURA_VERSION_CHECK ?= $(shell pkg-config --atleast-version=$(ZATHURA_MIN_VERSION) zathura; echo $$?)
ZATHURA_GTK_VERSION ?= $(shell pkg-config --variable=GTK_VERSION zathura)

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
DESKTOPPREFIX ?= ${PREFIX}/share/applications

# libs
CAIRO_INC ?= $(shell pkg-config --cflags cairo)
CAIRO_LIB ?= $(shell pkg-config --libs cairo)

LIBARCHIVE_INC ?= $(shell pkg-config --cflags libarchive)
LIBARCHIVE_LIB ?= $(shell pkg-config --libs libarchive)

GLIB_INC ?= $(shell pkg-config --cflags glib-2.0 gio-2.0)
GLIB_LIB ?= $(shell pkg-config --libs   glib-2.0 gio-2.0)

GDK_INC ?= $(shell pkg-config --cflags gdk-pixbuf-2.0 gdk-${ZATHURA_GTK_VERSION}.0)
GDK_LIB ?= $(shell pkg-config --libs   gdk-pixbuf-2.0 gdk-${ZATHURA_GTK_VERSION}.0)

GIRARA_INC ?= $(shell pkg-config --cflags girara-gtk${ZATHURA_GTK_VERSION})
GIRARA_LIB ?= $(shell pkg-config --libs girara-gtk${ZATHURA_GTK_VERSION})

ZATHURA_INC ?= $(shell pkg-config --cflags zathura)

INCS = ${GIRARA_INC} ${GDK_INC} ${GLIB_INC} ${ZATHURA_INC} ${LIBARCHIVE_INC}
LIBS = ${GIRARA_LIB} ${GDK_LIB} ${GLIB_LIB} ${LIBARCHIVE_LIB}

# plugindir
PLUGINDIR ?= $(shell pkg-config --variable=plugindir zathura)
ifeq (,${PLUGINDIR})
PLUGINDIR = ${LIBDIR}/zathura
endif

# flags
CFLAGS += -std=c11 -fPIC -pedantic -Wall -Wno-format-zero-length $(INCS)

# debug
DFLAGS ?= -g

# build with cairo support?
WITH_CAIRO ?= 1

# compiler
CC ?= gcc
LD ?= ld

# set to something != 0 if you want verbose build output
VERBOSE ?= 0
