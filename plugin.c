/* See LICENSE file for license and copyright information */

#include "plugin.h"

void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open     = (zathura_plugin_document_open_t) cb_document_open;
  functions->document_free     = (zathura_plugin_document_free_t) cb_document_free;
  functions->document_index_generate  = (zathura_plugin_document_index_generate_t) cb_document_index_generate;
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
