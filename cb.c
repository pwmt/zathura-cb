/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "cb.h"

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
  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_document_free(zathura_document_t* document, void* data)
{
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
