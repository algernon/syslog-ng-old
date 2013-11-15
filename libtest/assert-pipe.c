#include <logpipe.h>
#include "assert-pipe.h"
#include "testutils.h"

static char* 
assert_pipe_get_next_value(AssertPipe* self)
{
   char* value = self->current_assert->data;
   self->current_assert = g_list_next(self->current_assert);
   return value;
};

static int
assert_pipe_has_more_items(AssertPipe* self)
{
   return self->current_assert != NULL;
};

static void assert_pipe_queue(LogPipe* s, LogMessage* lm, const LogPathOptions *path_options)
{
   AssertPipe* self = (AssertPipe*) s;
   gssize value_len;
   const char *expected_name, *expected_value, *actual_value;

   assert_true(assert_pipe_has_more_items(self), "AssertPipe has no more items to assert!");
   expected_name = assert_pipe_get_next_value(self);
   expected_value = assert_pipe_get_next_value(self);
   NVHandle handle = log_msg_get_value_handle(expected_name);
   assert_true(handle != 0, "AssertPipe: no such value in this message!");
   actual_value = log_msg_get_value(lm, handle, &value_len);
   assert_string(expected_value, actual_value, "AssertPipe: Expected message value does not equals to actual one!");
};

void assert_pipe_init(AssertPipe* self, gchar* assert_key, gchar* assert_value, va_list va)
{ 
   char* key;
   char* value;
   self->super.queue = assert_pipe_queue;
   
   key = assert_key;
   value = assert_value;
   while (key)
   {
      self->asserts = g_list_append(self->asserts, key);
      self->asserts = g_list_append(self->asserts, value);
      key = va_arg(va, char*);
      value = va_arg(va, char*);
   };
   self->current_assert = self->asserts;
   
};

LogPipe* assert_pipe_new(char* assert_key, char* assert_value, ...)
{
   AssertPipe* self = g_new0(AssertPipe, 1);
   log_pipe_init_instance(&self->super);
   va_list va;
   va_start(va, assert_value);
   assert_pipe_init(self, assert_key, assert_value, va);
   va_end(va);
   return &self->super;
};


