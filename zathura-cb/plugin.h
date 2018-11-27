/* SPDX-License-Identifier: Zlib */

#ifndef CB_H
#define CB_H

#include <stdbool.h>
#include <cairo.h>

#include <girara/macros.h>

#include <zathura/plugin-api.h>

typedef struct cb_document_s cb_document_t;
typedef struct cb_page_s cb_page_t;

/**
 * Opens a new document
 *
 * @param document The document
 * @return ZATHURA_ERROR_OK if no error occurred
 */
GIRARA_HIDDEN zathura_error_t cb_document_open(zathura_document_t* document);

/**
 * Frees the document
 *
 * @param document The document
 * @param data Custom data
 * @return ZATHURA_ERROR_OK if no error occurred
 */
GIRARA_HIDDEN zathura_error_t cb_document_free(zathura_document_t* document, void* cb_document);

/**
 * Generates the index of the document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return Tree node object or NULL if an error occurred (e.g.: the document has
 *   no index)
 */
GIRARA_HIDDEN girara_tree_node_t* cb_document_index_generate(zathura_document_t* document,
    void* cb_document, zathura_error_t* error);

/**
 * Initializes a page
 *
 * @param page The page
 * @return ZATHURA_ERROR_OK if no error occurred
 */
zathura_error_t cb_page_init(zathura_page_t* page);

/**
 * Clear page
 *
 * @param page The page
 * @param cb_page cb Page
 * @return ZATHURA_ERROR_OK if no error occurred
 */
GIRARA_HIDDEN zathura_error_t cb_page_clear(zathura_page_t* page, void* cb_page);

/**
 * Renders the page to a cairo object
 *
 * @param page The page
 * @param cb_page cb Page
 * @param cairo Cairo object
 * @param printing Render for printing
 * @return ZATHURA_ERROR_OK if no error occurred
 */
GIRARA_HIDDEN zathura_error_t cb_page_render_cairo(zathura_page_t* page, void* cb_page,
    cairo_t* cairo, bool printing);

#endif // CB_H
