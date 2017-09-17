#include "cli_config.h"
#include "cli_microrl.h"


#define VER "1.0"
// definition commands word
#define _CMD_HELP   "help"
#define _CMD_INFO   "info"
#define _CMD_CLEAR  "clear"
#define _CMD_SETCH  "sch"

#define _NUM_OF_CMD 4
#define _NUM_OF_SETCLEAR_SCMD 2



void print(const char* str);
char get_char (void);
int execute (int argc, const char * const * argv);
char ** complet (int argc, const char * const * argv);
void sigint (void);
