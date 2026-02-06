/* SPDX-License-Identifier: Zlib */

#include <gio/gio.h>
#include <archive.h>
#include <archive_entry.h>
#include <string.h>

#include "plugin.h"
#include "internal.h"
#include "utils.h"

static GdkPixbuf* load_pixbuf_from_archive(zathura_page_t* page, const char* archive, const char* file) {
  if (archive == NULL || file == NULL) {
    return NULL;
  }

  if (g_file_test(archive, G_FILE_TEST_IS_DIR)) {
    return gdk_pixbuf_new_from_file(file, NULL);
  }

  cb_page_t* internal_page = zathura_page_get_data(page);
  if (internal_page->pixbuf) {
    return g_object_ref(internal_page->pixbuf);
  }

  g_autoptr(archive_t) a = archive_read_new();
  if (a == NULL) {
    return NULL;
  }

  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  int r = archive_read_open_filename(a, archive, LIBARCHIVE_BUFFER_SIZE);
  if (r != ARCHIVE_OK) {
    return NULL;
  }

  struct archive_entry* entry = NULL;
  while ((r = archive_read_next_header(a, &entry)) != ARCHIVE_EOF) {
    if (r < ARCHIVE_WARN) {
      return NULL;
    } else if (r == ARCHIVE_RETRY) {
      continue;
    }

    const char* path = archive_entry_pathname(entry);
    if (compare_path(path, file) != 0) {
      continue;
    }

    g_autoptr(GInputStream) is = g_memory_input_stream_new();
    if (is == NULL) {
      return NULL;
    }
    GMemoryInputStream* mis = G_MEMORY_INPUT_STREAM(is);

    uint8_t buf[LIBARCHIVE_BUFFER_SIZE];
    la_ssize_t bytes_read;
    while ((bytes_read = archive_read_data(a, buf, sizeof(buf))) != 0) {
      if (bytes_read < ARCHIVE_WARN) {
        return NULL;
      }

      void* tmp = g_memdup2(buf, bytes_read);
      if (tmp == NULL) {
        return NULL;
      }

      g_memory_input_stream_add_data(mis, tmp, bytes_read, g_free);
    }

    GdkPixbuf* pixbuf     = gdk_pixbuf_new_from_stream(is, NULL, NULL);
    internal_page->pixbuf = g_object_ref(pixbuf);
    return pixbuf;
  }

  return NULL;
}

zathura_error_t cb_page_render_cairo(zathura_page_t* page, void* data, cairo_t* cairo, bool UNUSED(printing)) {
  cb_page_t* cb_page = data;
  if (page == NULL || cb_page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  if (document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  g_autoptr(GdkPixbuf) pixbuf = load_pixbuf_from_archive(page, zathura_document_get_path(document), cb_page->file);
  if (pixbuf == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  gdk_cairo_set_source_pixbuf(cairo, pixbuf, 0, 0);
  cairo_paint(cairo);

  return ZATHURA_ERROR_OK;
}
