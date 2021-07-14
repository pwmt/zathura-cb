/* SPDX-License-Identifier: Zlib */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <girara/datastructures.h>
#include <archive.h>
#include <archive_entry.h>

#include "plugin.h"
#include "internal.h"
#include "utils.h"

static int compare_pages(const cb_document_page_meta_t* page1, const cb_document_page_meta_t* page2);
static bool read_archive(cb_document_t* cb_document, const char* archive, girara_list_t* supported_extensions);
static char* get_extension(const char* path);
static void cb_document_page_meta_free(cb_document_page_meta_t* meta);

zathura_error_t
cb_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  cb_document_t* cb_document = g_malloc0(sizeof(cb_document_t));

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
  cb_document->pages = girara_sorted_list_new2((girara_compare_function_t)
      compare_pages, (girara_free_function_t) cb_document_page_meta_free);
  if (cb_document->pages == NULL) {
    goto error_free;
  }

  /* read files recursively */
  if (read_archive(cb_document, path, supported_extensions) == false) {
    goto error_free;
  }

  girara_list_free(supported_extensions);

  /* set document information */
  zathura_document_set_number_of_pages(document, girara_list_size(cb_document->pages));
  zathura_document_set_data(document, cb_document);

  return ZATHURA_ERROR_OK;

error_free:

  girara_list_free(supported_extensions);
  cb_document_free(document, cb_document);

  return ZATHURA_ERROR_UNKNOWN;
}

zathura_error_t
cb_document_free(zathura_document_t* UNUSED(document), void* data)
{
  cb_document_t* cb_document = data;
  if (cb_document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  /* remove page list */
  if (cb_document->pages != NULL) {
    girara_list_free(cb_document->pages);
  }

  g_free(cb_document);

  return ZATHURA_ERROR_OK;
}

static void
cb_document_page_meta_free(cb_document_page_meta_t* meta)
{
  if (meta == NULL) {
    return;
  }

  if (meta->file != NULL) {
    g_free(meta->file);
  }
  g_free(meta);
}

static void
get_pixbuf_size(GdkPixbufLoader* loader, int width, int height, gpointer data)
{
  cb_document_page_meta_t* meta = (cb_document_page_meta_t*)data;

  meta->width = width;
  meta->height = height;

  gdk_pixbuf_loader_set_size(loader, 0, 0);
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
    char* extension = get_extension(path);

    if (extension == NULL) {
      continue;
    }

    GIRARA_LIST_FOREACH(supported_extensions, char*, iter, ext)
      if (g_strcmp0(extension, ext) == 0) {
        cb_document_page_meta_t* meta = g_malloc0(sizeof(cb_document_page_meta_t));
        meta->file = g_strdup(path);

        GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
        g_signal_connect(loader, "size-prepared", G_CALLBACK(get_pixbuf_size), meta);

        size_t size = 0;
        const void* buf = NULL;
        __LA_INT64_T offset = 0;
        while ((r = archive_read_data_block(a, &buf, &size, &offset)) != ARCHIVE_EOF) {
          if (r < ARCHIVE_WARN) {
            break;
          }

          if (buf == NULL || size <= 0) {
            continue;
          }

          if (gdk_pixbuf_loader_write(loader, buf, size, NULL) == false) {
            break;
          }

          if (meta->width > 0 && meta->height > 0) {
            break;
          }
        }

        gdk_pixbuf_loader_close(loader, NULL);
        g_object_unref(loader);

        if (meta->width > 0 && meta->height > 0) {
          girara_list_append(cb_document->pages, meta);
        } else {
          cb_document_page_meta_free(meta);
        }

        break;
      }
    GIRARA_LIST_FOREACH_END(supported_extensions, char*, iter, ext);

    g_free(extension);
  }

  archive_read_close(a);
  archive_read_free(a);
  return true;
}

static int
compare_pages(const cb_document_page_meta_t* page1, const cb_document_page_meta_t* page2)
{
  return compare_path(page1->file, page2->file);
}

static char*
get_extension(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  const char* res = strrchr(path, '.');
  if (res == NULL) {
    return NULL;
  }

  return g_ascii_strdown(res + 1, -1);
}
