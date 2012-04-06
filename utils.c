/* See LICENSE file for license and copyright information */

#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>

#include "utils.h"

char*
get_mime_type(const char* path)
{
  char* uri            = g_filename_to_uri(path, NULL, NULL);
  GFile* file          = g_file_new_for_uri(uri);
  GFileInfo* file_info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, 0, NULL, NULL);

  g_object_unref(file);

  if (file_info == NULL) {
    return NULL;
  }

  const char* content_type = g_file_info_get_content_type(file_info);
  char* mime_type          = NULL;

  if (content_type != NULL) {
    mime_type = g_content_type_get_mime_type(content_type);
  }

  g_object_unref(file_info);

  return mime_type;
}
