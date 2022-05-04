/* SPDX-License-Identifier: Zlib */

#ifndef UTILS_H
#define UTILS_H

#include <girara/macros.h>

/**
 * Compares two paths with each other
 *
 * @param str1 First path
 * @param str2 Second path
 *
 * @return   
 */
GIRARA_HIDDEN int compare_path(const char* str1, const char* str2);
/**
 * Checks if a file is a directory
 * 
 * @param path The filepath
 */
int is_dir(const char* path);

#endif // UTILS_H
