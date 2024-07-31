#ifndef COMMANDS_H_   /* Include guard */
#define COMMANDS_H_

#include "shellper.h"

char* get_cur_dir(void);
State* exec_cd(State* s);
void print_history(char** history);

#endif
