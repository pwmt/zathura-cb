/* See LICENSE file for license and copyright information */

#include <glib.h>

#include "utils.h"

int
compare_path(const char* str1, const char* str2)
{
  char* ustr1 = g_utf8_casefold(str1, -1);
  char* ustr2 = g_utf8_casefold(str2, -1);
  int result  = g_utf8_collate(ustr1, ustr2);
  g_free(ustr1);
  g_free(ustr2);

  return result;
}
