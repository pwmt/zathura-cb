/* See LICENSE file for license and copyright information */

#ifndef cb_H
#define cb_H

#include <stdbool.h>

#if HAVE_CAIRO
#include <cairo.h>
#endif

#include <zathura/plugin-api.h>

typedef struct cb_page_s cb_page_t;

/**
 * Opens a new document
 *
 * @param document The document
 * @return ZATHURA_ERROR_OK if no error occured
 */
zathura_error_t cb_document_open(zathura_document_t* document);

/**
 * Frees the document
 *
 * @param document The document
 * @param data Custom data
 * @return ZATHURA_ERROR_OK if no error occured
 */
zathura_error_t cb_document_free(zathura_document_t* document, void* data);

/**
 * Initializes a page
 *
 * @param page The page
 * @return ZATHURA_ERROR_OK if no error occured
 */
zathura_error_t cb_page_init(zathura_page_t* page);

/**
 * Clear page
 *
 * @param page The page
 * @param cb_page cb Page
 * @return ZATHURA_ERROR_OK if no error occured
 */
zathura_error_t cb_page_clear(zathura_page_t* page, cb_page_t* cb_page);

#if HAVE_CAIRO
/**
 * Renders the page to a cairo object
 *
 * @param page The page
 * @param cb_page cb Page
 * @param cairo Cairo object
 * @param printing Render for printing
 * @return ZATHURA_ERROR_OK if no error occured
 */
zathura_error_t cb_page_render_cairo(zathura_page_t* page, cb_page_t* cb_page,
    cairo_t* cairo, bool printing);
#endif

#endif // cb_H
