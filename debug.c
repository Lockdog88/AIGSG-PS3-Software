#include "debug.h"

void debug_str(char *str)
{
  if (DEBUG_F) QM_PRINTF("%s\r\n", str);
}
