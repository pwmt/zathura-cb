/* SPDX-License-Identifier: Zlib */

#include <gio/gio.h>
#include <archive.h>
#include <archive_entry.h>
#include <gtk/gtk.h>
#include <string.h>

#include "plugin.h"
#include "internal.h"
#include "utils.h"

static GdkPixbuf* load_pixbuf_from_archive(const char* archive, const char* file);

zathura_error_t
cb_page_render_cairo(zathura_page_t* page, void* data,
    cairo_t* cairo, bool UNUSED(printing))
{
  cb_page_t* cb_page = data;
  if (page == NULL || cb_page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  if (document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  GdkPixbuf* pixbuf = load_pixbuf_from_archive(zathura_document_get_path(document), cb_page->file);
  if (pixbuf == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  gdk_cairo_set_source_pixbuf(cairo, pixbuf, 0, 0);
  cairo_paint(cairo);
  g_object_unref(pixbuf);

  return ZATHURA_ERROR_OK;
}

static GdkPixbuf*
load_pixbuf_from_archive(const char* archive, const char* file)
{
  if (archive == NULL || file == NULL) {
    return NULL;
  }

  struct archive* a = archive_read_new();
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
      archive_read_close(a);
      archive_read_free(a);
      return NULL;
    }

    const char* path = archive_entry_pathname(entry);
    if (compare_path(path, file) != 0) {
      continue;
    }

    GInputStream* is = g_memory_input_stream_new();
    if (is == NULL) {
      archive_read_close(a);
      archive_read_free(a);
      return NULL;
    }
    GMemoryInputStream* mis = G_MEMORY_INPUT_STREAM(is);

    size_t size = 0;
    const void* buf = NULL;
    __LA_INT64_T offset = 0;
    while ((r = archive_read_data_block(a, &buf, &size, &offset)) != ARCHIVE_EOF) {
      if (r < ARCHIVE_WARN) {
        archive_read_close(a);
        archive_read_free(a);
        g_object_unref(mis);
        return NULL;
      }

      if (size == 0 || buf == NULL) {
        continue;
      }

      void* tmp = g_malloc0(size);
      if (tmp == NULL) {
        archive_read_close(a);
        archive_read_free(a);
        g_object_unref(mis);
        return NULL;
      }

      memcpy(tmp, buf, size);
      g_memory_input_stream_add_data(mis, tmp, size, g_free);
    }

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(is, NULL, NULL);
    if (pixbuf == NULL) {
      archive_read_close(a);
      archive_read_free(a);
      g_object_unref(mis);
      return NULL;
    }

    archive_read_close(a);
    archive_read_free(a);
    g_object_unref(mis);
    return pixbuf;
  }

  archive_read_close(a);
  archive_read_free(a);
  return NULL;
}

