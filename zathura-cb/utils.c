/* SPDX-License-Identifier: Zlib */

#include "utils.h"

#include <archive.h>
#include <glib.h>

int compare_path(const char* str1, const char* str2) {
  g_autofree char* ustr1 = g_utf8_casefold(str1, -1);
  g_autofree char* ustr2 = g_utf8_casefold(str2, -1);

  g_autofree char* ustr3 = g_utf8_collate_key_for_filename(ustr1, -1);
  g_autofree char* ustr4 = g_utf8_collate_key_for_filename(ustr2, -1);

  return g_strcmp0(ustr3, ustr4);
}

void libarchive_archive_free(void* data) {
  if (data != NULL) {
    archive_t* archive = data;
    archive_read_free(archive);
  }
}
