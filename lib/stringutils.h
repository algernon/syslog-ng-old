#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H
#include "syslog-ng.h"

gchar* data_to_hex_string(guint8 *data, guint32 length);
gchar* replace_string(const gchar *source, const gchar *substring, const gchar *replacement);

#endif

