/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

/**
 * Returns the mime type of the given file
 *
 * @param path The path to the file
 * @return The mime type or NULL
 */
char* get_mime_type(const char* path);

/**
 * Extracts an archive to a directory
 *
 * @param archive The archive
 * @param directory The target directory
 * @return true if no error occured
 */
bool extract_archive_to_directory(const char* archive, const char* directory);

/**
 * Returns the file extension of a path
 *
 * @param path Path to the file
 * @return The file extension or NULL
 */
const char* file_get_extension(const char* path);

#endif // UTILS_H
