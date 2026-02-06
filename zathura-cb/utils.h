/* SPDX-License-Identifier: Zlib */

#ifndef UTILS_H
#define UTILS_H

#include <girara/macros.h>
#include <glib.h>

/**
 * Compares two paths with each other
 *
 * @param str1 First path
 * @param str2 Second path
 *
 * @return
 */
GIRARA_HIDDEN int compare_path(const char* str1, const char* str2);

#if defined(BUFSIZ)
#define BUFFER_SIZE BUFSIZ
#else
#define BUFFER_SIZE 4098
#endif

typedef struct archive archive_t;

void libarchive_archive_free(void* archive);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(archive_t, libarchive_archive_free)

#endif // UTILS_H
