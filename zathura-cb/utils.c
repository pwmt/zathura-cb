/* SPDX-License-Identifier: Zlib */

#include <glib.h>

#include "utils.h"

int
compare_path(const char* str1, const char* str2)
{
  char* ustr1 = g_utf8_casefold(str1, -1);
  char* ustr2 = g_utf8_casefold(str2, -1);

  char* ustr3 = g_utf8_collate_key_for_filename(ustr1, -1);
  char* ustr4 = g_utf8_collate_key_for_filename(ustr2, -1);
  g_free(ustr2);
  g_free(ustr1);

  int result  = g_strcmp0(ustr3, ustr4);
  g_free(ustr4);
  g_free(ustr3);

  return result;
}
