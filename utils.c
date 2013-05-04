/* See LICENSE file for license and copyright information */

#include <gio/gio.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>

#include <archive.h>
#include <archive_entry.h>

#include "utils.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define LIBARCHIVE_BUFFER_SIZE 8192

static int
count_archive_files(const char *archive_path)
{
    int r, file_fount;
    struct archive *a;
    struct archive_entry *entry;

    if((a = archive_read_new()) == NULL) {
        printf("Error: an not open archive!\n");
        return -1;
    }

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    r = archive_read_open_filename(a, archive_path, (size_t) LIBARCHIVE_BUFFER_SIZE);
    if (r != ARCHIVE_OK) {
        printf("Error: %s\n", archive_error_string(a));
        goto cleanup;
    }

    while(archive_read_next_header(a, &entry) == ARCHIVE_OK);
    file_fount = archive_file_count(a);

    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        printf("Error: %s\n", archive_error_string(a));
        return -1;
    }

    return file_fount;

cleanup:
    printf("Error: %s\n", archive_error_string(a));
    archive_read_free(a);
    return -1;
}

bool
extract_archive_to_directory(const char* archive_path, const char* directory)
{
  if (archive_path == NULL || directory == NULL) {
    return false;
  }

  int r, file_count;
  struct archive *a;
  struct archive_entry *entry;
  file_count = count_archive_files(archive_path);

  if((a = archive_read_new()) == NULL) {
    printf("Error: an not open archive!\n");
    return false;
  }

  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);

  r = archive_read_open_filename(a, archive_path, (size_t) LIBARCHIVE_BUFFER_SIZE);
  if (r != ARCHIVE_OK) {
    printf("Error: %s\n", archive_error_string(a));
    goto cleanup;
  }

  chdir(directory);  // TODO: Error reporting

  while(archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    // (*set_progress)((int) (archive_file_count(a) / (file_count / 100.0)));
    if (archive_read_extract(a, entry, 0) != ARCHIVE_OK) {
      printf("Error: %s\n", archive_error_string(a));
      goto cleanup;
    }
  }

  r = archive_read_free(a);
  if (r != ARCHIVE_OK) {
    printf("Error: %s\n", archive_error_string(a));
    return false;
  }

  return true;

cleanup:
  printf("Error: %s\n", archive_error_string(a));
  archive_read_free(a);
  return false;
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
