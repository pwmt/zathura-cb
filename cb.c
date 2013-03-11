/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <girara/datastructures.h>

#include "cb.h"
#include "utils.h"

struct cb_document_s {
  char* directory; /**< Path to the directory */
  girara_list_t* page_paths; /**< List of page paths */
};

struct cb_page_s {
  char* file; /**< File name */
  GdkPixbuf* pixbuf; /**< Pixbuffer */
};

static int compare_path(const char* str1, const char* str2);
static zathura_error_t read_archive(cb_document_t* cb_document, const char* directory, girara_list_t* supported_extensions);

void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open     = (zathura_plugin_document_open_t) cb_document_open;
  functions->document_free     = (zathura_plugin_document_free_t) cb_document_free;
  functions->page_init         = (zathura_plugin_page_init_t) cb_page_init;
  functions->page_clear        = (zathura_plugin_page_clear_t) cb_page_clear;
  functions->page_render_cairo = (zathura_plugin_page_render_cairo_t) cb_page_render_cairo;
}

ZATHURA_PLUGIN_REGISTER(
  "cb",
  0, 2, 0,
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

  /* create temp directory */
  cb_document->directory = g_dir_make_tmp("zathura-cb-XXXXXX", NULL);
  if (cb_document->directory == NULL) {
    goto error_free;
  }

  /* extract files */
  const char* path = zathura_document_get_path(document);

  if (extract_archive_to_directory(path, cb_document->directory) == false) {
    goto error_free;
  }

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
  read_archive(cb_document, cb_document->directory, supported_extensions);

  girara_list_free(supported_extensions);

  /* set document information */
  zathura_document_set_number_of_pages(document, girara_list_size(cb_document->page_paths));
  zathura_document_set_data(document, cb_document);

  return ZATHURA_ERROR_OK;

error_free:

  cb_document_free(document, cb_document);

  return ZATHURA_ERROR_UNKNOWN;
}

static zathura_error_t
read_archive(cb_document_t* cb_document, const char* directory, girara_list_t* supported_extensions)
{
  GDir* dir = g_dir_open(directory, 0, NULL);
  if (dir == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  char* name = NULL;
  while ((name = (char*) g_dir_read_name(dir)) != NULL) {
    char* path = g_build_filename(directory, name, NULL);

    if (g_file_test(path, G_FILE_TEST_IS_SYMLINK) == TRUE) {
      g_free(path);
    } else if (g_file_test(path, G_FILE_TEST_IS_REGULAR) == TRUE) {
      const char* extension = file_get_extension(path);

      bool supported = false;
      GIRARA_LIST_FOREACH(supported_extensions, char*, iter, ext)
        if (g_strcmp0(extension, ext) == 0) {
          girara_list_append(cb_document->page_paths, path);
          supported = true;
          break;
        }
      GIRARA_LIST_FOREACH_END(supported_extensions, char*, iter, ext);

      if (supported == false) {
        g_free(path);
      }
    } else if (g_file_test(path, G_FILE_TEST_IS_DIR) == TRUE) {
      read_archive(cb_document, path, supported_extensions);
      g_free(path);
    } else {
      g_free(path);
    }
  }

  g_dir_close(dir);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_document_free(zathura_document_t* document, cb_document_t* cb_document)
{
  if (cb_document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  /* remove temp directory */
  if (cb_document->directory != NULL) {
    g_remove(cb_document->directory);
  }

  /* remove page list */
  if (cb_document->page_paths != NULL) {
    girara_list_free(cb_document->page_paths);
  }

  if (cb_document->directory != NULL) {
    g_free(cb_document->directory);
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

  /* extract dimensions */
  int width, height;
  GdkPixbufFormat* format = gdk_pixbuf_get_file_info(cb_page->file, &width, &height);
  if (format == NULL) { /* format not recognized */
    g_free(cb_page);
    return ZATHURA_ERROR_UNKNOWN;
  }

  zathura_page_set_width(page, width);
  zathura_page_set_height(page, height);
  zathura_page_set_data(page, cb_page);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_clear(zathura_page_t* page, cb_page_t* cb_page)
{
  if (cb_page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (cb_page->pixbuf != NULL) {
    g_object_unref(cb_page->pixbuf);
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

  if (cb_page->pixbuf == NULL) {
    cb_page->pixbuf = gdk_pixbuf_new_from_file(cb_page->file, NULL);
    if (cb_page->pixbuf == NULL) {
      return ZATHURA_ERROR_UNKNOWN;
    }
  }

  gdk_cairo_set_source_pixbuf(cairo, cb_page->pixbuf, 0, 0);
  cairo_paint(cairo);

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
