#include "stringutils.h"

gchar *
data_to_hex_string(guint8 *data, guint32 length)
{
  gchar *string = NULL;
  gchar *pstr = NULL;
  guint8 *tdata = data;
  guint32 i = 0;
  static const gchar hex_char[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

  if (!length)
    return NULL;

  string = (gchar *)g_malloc0(sizeof(gchar)*length*3 + 1);

  pstr = string;

  *pstr = hex_char[*tdata >> 4];
  pstr++;
  *pstr = hex_char[*tdata & 0x0F];

  for (i = 1; i < length; i++)
    {
      pstr++;
      *pstr = ' ';
      pstr++;
      *pstr = hex_char[tdata[i] >> 4];
      pstr++;
      *pstr = hex_char[tdata[i] & 0x0F];
    }
  pstr++;
  *pstr = '\0';
  return string;
}

gchar *
replace_string(const gchar *source, const gchar *substring, const gchar *replacement)
{
  gchar *p;
  GString *result = g_string_sized_new(1024);
  for(p = strstr(source, substring); p; source = p + strlen(substring), p = strstr(source, substring))
    {
      result = g_string_append_len(result, source, p - source);
      result = g_string_append(result, replacement);
    }
  result = g_string_append(result, source);
  return g_string_free(result, FALSE);
}

