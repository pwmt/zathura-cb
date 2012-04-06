/* See LICENSE file for license and copyright information */

#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#include <stdio.h>

#include "utils.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct archive_type_s {
  const char* mime_type;
  const char* application;
  const char* command;
} archive_type_t;

static const archive_type_t archive_types[] = {
  { "application/x-cbr",           "unrar", "%s e %s %s" },
  { "application/x-rar",           "unrar", "%s e %s %s" },
  { "application/x-cbz",           "unzip", "%s %s -d %s" },
  { "application/zip",             "unzip", "%s %s -d %s" },
  { "application/x-cb7",           "7z",    "%s e %s -o %s" },
  { "application/x-7z-compressed", "7z",    "%s e %s -o %s" },
  { "application/x-cbt",           "tar"    "%s -x -f %s -C %s" },
  { "application/x-tar",           "tar",   "%s -x -f %s -C %s" }
};

char*
get_mime_type(const char* path)
{
  char* uri            = g_filename_to_uri(path, NULL, NULL);
  GFile* file          = g_file_new_for_uri(uri);
  GFileInfo* file_info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, 0, NULL, NULL);

  g_free(uri);
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

bool
extract_archive_to_directory(const char* archive, const char* directory)
{
  if (archive == NULL || directory == NULL) {
    return false;
  }

  /* search supported archive types */
  char* mime_type   = get_mime_type(archive);
  char* executeable = NULL;

  unsigned int i = 0;
  for (i = 0; i < LENGTH(archive_types); i++) {
    if (strcmp(mime_type, archive_types[i].mime_type) == 0) {
      executeable = g_find_program_in_path(archive_types[i].application);
      if (executeable != NULL) {
        break;
      }
    }
  }

  /* there is no way to extract the archive */
  if (executeable == NULL) {
    return false;
  }

  /* build command */
  char* quoted_archive   = g_shell_quote(archive);
  char* quoted_directory = g_shell_quote(directory);

  char* command = g_strdup_printf(archive_types[i].command, executeable, quoted_archive, quoted_directory);

  g_free(quoted_archive);
  g_free(quoted_directory);
  g_free(executeable);

  /* run command */
  gboolean success = g_spawn_command_line_sync(command, NULL, NULL, NULL, NULL);

  g_free(command);

  return (success == TRUE) ? true : false;
}

const char*
file_get_extension(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  unsigned int i = strlen(path);
  for (; i > 0; i--) {
    if (*(path + i) != '.') {
      continue;
    } else {
      break;
    }
  }

  if (i != 0) {
    return NULL;
  }

  return path + i + 1;
}
