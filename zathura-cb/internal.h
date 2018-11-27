/* SPDX-License-Identifier: Zlib */

#ifndef INTERNAL_H
#define INTERNAL_H

#define LIBARCHIVE_BUFFER_SIZE 8192 

struct cb_document_s {
  girara_list_t* pages; /**< List of metadata structs */
};

struct cb_page_s {
  char* file; /**< Image associated to the page */
};

/** Image meta-data read during the document initialization
 */
typedef struct cb_document_page_meta_s {
  char* file; /**< Image file */
  int width; /**< Image width */
  int height; /**< Image height */
} cb_document_page_meta_t;

#endif // INTERNAL_H
