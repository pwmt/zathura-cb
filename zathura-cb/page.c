/* SPDX-License-Identifier: Zlib */

#include <glib.h>
#include <gtk/gtk.h>

#include "plugin.h"
#include "internal.h"

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

  cb_document_page_meta_t* meta = girara_list_nth(cb_document->pages, zathura_page_get_index(page));
  if (meta == NULL || meta->file == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  cb_page_t* cb_page = g_malloc0(sizeof(cb_page_t));
  if (cb_page == NULL) {
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  cb_page->file = g_strdup(meta->file);
  zathura_page_set_width(page, meta->width);
  zathura_page_set_height(page, meta->height);
  zathura_page_set_data(page, cb_page);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
cb_page_clear(zathura_page_t* UNUSED(page), void* data)
{
  cb_page_t* cb_page = data;
  if (cb_page == NULL) {
    return ZATHURA_ERROR_OK;
  }

  g_free(cb_page->file);
  g_free(cb_page);

  return ZATHURA_ERROR_OK;
}
