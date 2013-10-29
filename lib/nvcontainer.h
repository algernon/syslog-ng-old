#ifndef _NVCONTAINER_H
#define _NVCONTAINER_H
#include "syslog-ng.h"

typedef struct _NameValueContainer NameValueContainer;

NameValueContainer *name_value_container_new();

gboolean name_value_container_parse_json_string(NameValueContainer *self, gchar *json_string);

void name_value_container_add_string(NameValueContainer *self, gchar *name, gchar *value);
void name_value_container_add_boolean(NameValueContainer *self, gchar *name, gboolean value);
void name_value_container_add_int(NameValueContainer *self, gchar *name, gint value);
void name_value_container_add_int64(NameValueContainer *self, gchar *name, gint64 value);
gchar *name_value_container_get_value(NameValueContainer *self, gchar *name);

const gchar *name_value_container_get_string(NameValueContainer *self, gchar *name);
gboolean name_value_container_get_boolean(NameValueContainer *self, gchar *name);
gint name_value_container_get_int(NameValueContainer *self, gchar *name);
gint64 name_value_container_get_int64(NameValueContainer *self, gchar *name);

gchar *name_value_container_get_json_string(NameValueContainer *self);
void name_value_container_foreach(NameValueContainer *self, void
(callback)(gchar *name, const gchar *value, gpointer user_data), gpointer user_data);

gboolean  name_value_container_is_name_exists(NameValueContainer *self, gchar *name);
void name_value_container_free(NameValueContainer *self);

#endif

