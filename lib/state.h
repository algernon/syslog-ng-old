#ifndef STATE_H
#define STATE_H 1
#include "syslog-ng.h"
#include "persist-state.h"
#include "nvcontainer.h"

#define RPCTID_PREFIX "next.rcptid"

typedef struct _BaseState
{
  guint8 version;
  guint8 big_endian :1;
} BaseState;

typedef struct _StateHandler StateHandler;

struct _StateHandler
{
  gpointer state;
  gchar *name;
  gsize size;
  guint8 version;
  PersistEntryHandle persist_handle;
  PersistState *persist_state;
  NameValueContainer* (*dump_state)(StateHandler *self);
  PersistEntryHandle (*alloc_state)(StateHandler *self);
  gboolean (*load_state)(StateHandler *self, NameValueContainer *new_state, GError **error);
  void (*free_fn)(StateHandler *self);
};

PersistEntryHandle state_handler_get_handle(StateHandler *self);

gchar *state_handler_get_persist_name(StateHandler *self);

guint8 state_handler_get_persist_version(StateHandler *self);

gpointer state_handler_get_state(StateHandler *self);

PersistEntryHandle state_handler_alloc_state(StateHandler *self);

void state_handler_put_state(StateHandler *self);

gboolean state_handler_load_state(StateHandler *self, NameValueContainer *new_state, GError **error);

gboolean state_handler_rename_entry(StateHandler *self, const gchar *new_name);

gboolean state_handler_entry_exists(StateHandler *self);

NameValueContainer *state_handler_dump_state(StateHandler *self);

void state_handler_free(StateHandler *self);

void state_handler_init(StateHandler *self, PersistState *persist_state, const gchar *name);
typedef StateHandler* (*STATE_HANDLER_CONSTRUCTOR)(PersistState *persist_state, const gchar *name);

STATE_HANDLER_CONSTRUCTOR state_handler_get_constructor_by_prefix(const gchar *prefix);
void state_handler_register_constructor(const gchar *prefix,
    STATE_HANDLER_CONSTRUCTOR handler);

void state_handler_register_default_constructors();

#endif
