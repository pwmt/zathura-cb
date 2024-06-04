/* SPDX-License-Identifier: Zlib */

#include "plugin.h"
#include "internal.h"
#include <glib.h>

girara_tree_node_t* cb_document_index_generate(zathura_document_t* document, void* data, zathura_error_t* error) {
  cb_document_t* cb_document = data;
  if (document == NULL || cb_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  girara_tree_node_t* root = girara_node_new(zathura_index_element_new("ROOT"));
  for (unsigned int page_number = 0; page_number < girara_list_size(cb_document->pages); ++page_number) {
    cb_document_page_meta_t* page          = girara_list_nth(cb_document->pages, page_number);
    gchar* markup                          = g_markup_escape_text(page->file, -1);
    zathura_index_element_t* index_element = zathura_index_element_new(markup);
    g_free(markup);

    if (index_element != NULL) {
      zathura_rectangle_t rect     = {0, 0, 0, 0};
      zathura_link_target_t target = {ZATHURA_LINK_DESTINATION_XYZ, NULL, page_number, -1, -1, -1, -1, 0};

      index_element->link = zathura_link_new(ZATHURA_LINK_GOTO_DEST, rect, target);
      girara_node_append_data(root, index_element);
    }
  }

  return root;
}
