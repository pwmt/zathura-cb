/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "cb.h"
#include "utils.h"

struct cb_document_s {
  char* directory; /**< Path to the directory */
};

struct cb_page_s {
  char* file; /**< File name */
};

void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open     = cb_document_open;
  functions->document_free     = cb_document_free;
  functions->page_init         = cb_page_init;
  functions->page_clear        = cb_page_clear;
  functions->page_render_cairo = cb_page_render_cairo;
}

ZATHURA_PLUGIN_REGISTER(
  "cb",
  0, 1, 0,
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
    cb_document_free(document, cb_document);
    return ZATHURA_ERROR_UNKNOWN;
  }

  /* extract file */
  const char* path = zathura_document_get_path(document);
  char* mime_type  = get_mime_type(path);

  fprintf(stderr, "mime_type: %s\n", mime_type);

  zathura_document_set_number_of_pages(document, 1);
  zathura_document_set_data(document, cb_document);

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

  g_free(cb_document);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_init(zathura_page_t* page)
{
  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_clear(zathura_page_t* page, cb_page_t* cb_page)
{
  return ZATHURA_ERROR_OK;
}

#if HAVE_CAIRO
zathura_error_t
cb_page_render_cairo(zathura_page_t* page, cb_page_t* cb_page,
    cairo_t* cairo, bool printing)
{
  return ZATHURA_ERROR_OK;
}
#endif
