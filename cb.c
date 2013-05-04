/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <girara/datastructures.h>
#include <gio/gio.h>
#include <archive.h>
#include <archive_entry.h>
#include <string.h>

#define LIBARCHIVE_BUFFER_SIZE 8192

#include "cb.h"

struct cb_document_s {
  girara_list_t* page_paths; /**< List of page paths */
};

struct cb_page_s {
  char* file; /**< File name */
};

static int compare_path(const char* str1, const char* str2);
static bool read_archive(cb_document_t* cb_document, const char* archive, girara_list_t* supported_extensions);
static GdkPixbuf* load_pixbuf_from_archive(const char* archive, const char* file);
static const char* get_extension(const char* path);

void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open     = (zathura_plugin_document_open_t) cb_document_open;
  functions->document_free     = (zathura_plugin_document_free_t) cb_document_free;
  functions->page_init         = (zathura_plugin_page_init_t) cb_page_init;
  functions->page_clear        = (zathura_plugin_page_clear_t) cb_page_clear;
#ifdef HAVE_CAIRO
  functions->page_render_cairo = (zathura_plugin_page_render_cairo_t) cb_page_render_cairo;
#endif
}

ZATHURA_PLUGIN_REGISTER(
  "cb",
  VERSION_MAJOR, VERSION_MINOR, VERSION_REV,
  register_functions,
  ZATHURA_PLUGIN_MIMETYPES({
    "application/x-cbr",
    "application/x-rar",
    "application/x-cbz",
    "application/zip",
    "application/x-cb7",
    "application/x-7z-compressed",
    "application/x-cbt",
    "application/x-tar"
  })
)

zathura_error_t
cb_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  cb_document_t* cb_document = g_malloc0(sizeof(cb_document));

  /* archive path */
  const char* path = zathura_document_get_path(document);

  /* create list of supported formats */
  girara_list_t* supported_extensions = girara_list_new2(g_free);
  if (supported_extensions == NULL) {
    goto error_free;
  }

  GSList* formats = gdk_pixbuf_get_formats();
  for (GSList* list  = formats; list != NULL; list = list->next) {
    GdkPixbufFormat* format = (GdkPixbufFormat*) list->data;
    char** extensions = gdk_pixbuf_format_get_extensions(format);

    for (unsigned int i = 0; extensions[i] != NULL; i++) {
      girara_list_append(supported_extensions, g_strdup(extensions[i]));
    }

    g_strfreev(extensions);
  }
  g_slist_free(formats);

  /* create list of supported files (pages) */
  cb_document->page_paths = girara_sorted_list_new2((girara_compare_function_t)
      compare_path, (girara_free_function_t) g_free);
  if (cb_document->page_paths == NULL) {
    goto error_free;
  }

  /* read files recursively */
  if (read_archive(cb_document, path, supported_extensions) == false) {
    goto error_free;
  }

  girara_list_free(supported_extensions);

  /* set document information */
  zathura_document_set_number_of_pages(document, girara_list_size(cb_document->page_paths));
  zathura_document_set_data(document, cb_document);

  return ZATHURA_ERROR_OK;

error_free:

  cb_document_free(document, cb_document);

  return ZATHURA_ERROR_UNKNOWN;
}

static bool
read_archive(cb_document_t* cb_document, const char* archive, girara_list_t* supported_extensions)
{
  struct archive* a = archive_read_new();
  if (a == NULL) {
    return false;
  }

  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  int r = archive_read_open_filename(a, archive, (size_t) LIBARCHIVE_BUFFER_SIZE);
  if (r != ARCHIVE_OK) {
    archive_read_free(a);
    return false;
  }

  struct archive_entry *entry = NULL;
  while ((r = archive_read_next_header(a, &entry)) != ARCHIVE_EOF) {
    if (r < ARCHIVE_WARN) {
      // let's ignore warnings ... they are non-fatal errors
      archive_read_close(a);
      archive_read_free(a);
      return false;
    }

    if (archive_entry_filetype(entry) != AE_IFREG) {
      // we only care about regular files
      continue;
    }

    const char* path = archive_entry_pathname(entry);
    const char* extension = get_extension(path);

    GIRARA_LIST_FOREACH(supported_extensions, char*, iter, ext)
      if (g_strcmp0(extension, ext) == 0) {
        girara_list_append(cb_document->page_paths, g_strdup(path));
        break;
      }
    GIRARA_LIST_FOREACH_END(supported_extensions, char*, iter, ext);
  }

  archive_read_close(a);
  archive_read_free(a);
  return true;
}

zathura_error_t
cb_document_free(zathura_document_t* document, cb_document_t* cb_document)
{
  if (cb_document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  /* remove page list */
  if (cb_document->page_paths != NULL) {
    girara_list_free(cb_document->page_paths);
  }

  g_free(cb_document);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  cb_document_t* cb_document   = zathura_document_get_data(document);

  if (document == NULL || cb_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  cb_page_t* cb_page = g_malloc0(sizeof(cb_page_t));
  cb_page->file = girara_list_nth(cb_document->page_paths, zathura_page_get_index(page));

  GdkPixbuf* pixbuf = load_pixbuf_from_archive(zathura_document_get_path(document), cb_page->file);
  if (pixbuf == NULL) {
    g_free(cb_page);
    return ZATHURA_ERROR_UNKNOWN;
  }

  /* extract dimensions */
  zathura_page_set_width(page, gdk_pixbuf_get_width(pixbuf));
  zathura_page_set_height(page, gdk_pixbuf_get_height(pixbuf));
  zathura_page_set_data(page, cb_page);
  g_object_unref(pixbuf);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_clear(zathura_page_t* page, cb_page_t* cb_page)
{
  if (cb_page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  g_free(cb_page);

  return ZATHURA_ERROR_OK;
}

#if HAVE_CAIRO
zathura_error_t
cb_page_render_cairo(zathura_page_t* page, cb_page_t* cb_page,
    cairo_t* cairo, bool printing)
{
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
#endif

static int
compare_path(const char* str1, const char* str2)
{
  char* ustr1 = g_utf8_casefold(str1, -1);
  char* ustr2 = g_utf8_casefold(str2, -1);
  int result  = g_utf8_collate(ustr1, ustr2);
  g_free(ustr1);
  g_free(ustr2);

  return result;
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
    if (compare_path(path, file) != 0)
      continue;

    GInputStream* is = g_memory_input_stream_new();
    if (is == NULL) {
      archive_read_close(a);
      archive_read_free(a);
      return NULL;
    }
    GMemoryInputStream* mis = G_MEMORY_INPUT_STREAM(is);

    size_t size = 0;
    const void* buf = NULL;
    off_t offset = 0;
    while ((r = archive_read_data_block(a, &buf, &size, &offset)) != ARCHIVE_EOF) {
      if (r < ARCHIVE_WARN) {
        archive_read_close(a);
        archive_read_free(a);
        g_object_unref(mis);
        return NULL;
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

static const char*
get_extension(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  const char* res = strrchr(path, '.');
  if (res == NULL) {
    return NULL;
  }

  return res + 1;
}
