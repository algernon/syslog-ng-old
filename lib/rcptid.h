#ifndef _RCPTID_H
#define _RCPTID_H

#include "state.h"

StateHandler* create_state_handler_rcptid(PersistState *persist_state, const gchar *name);

typedef struct _RcptidState
{
  BaseState super;
  guint64 g_rcptid;
} RcptidState;

gboolean rcptid_init(PersistState *state);
void log_msg_create_rcptid(LogMessage *msg);
#endif
