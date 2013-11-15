#include <logpipe.h>

typedef struct _AssertPipe {
   LogPipe super;
   GList* asserts;
   GList* current_assert;
} AssertPipe;

LogPipe* assert_pipe_new(char* name, char* value, ...);
