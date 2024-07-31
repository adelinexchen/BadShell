#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include "shellper.h"
#include "commands.h"

#define MAX_BUFF 1000


char* get_cur_dir(void) {
    char cwd[MAX_BUFF];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        exit(1);
    }

    char* delim = "/";
    char* lastToken = NULL;
    char* token = strtok(cwd, delim);

    // loop through the string to extract all other tokens
    while( token != NULL ) {
        //printf( " %s\n", token ); // printing each token
        lastToken = token;
        token = strtok(NULL, delim);
    }

    char* curdir = strdup(lastToken);
    return curdir;
}


State* exec_cd(State* s) {
    char path[MAX_BUFF];
    // e.g. cd DIRECTORY blah blah
    if (array_length(s->args) > 2) {
        fprintf(stderr, "too many arguments...");
        return s;
    }

    char* arg = s->args[1];
    char curdir[MAX_BUFF];
    // no cur dir
    if (getcwd(curdir, sizeof(curdir)) == NULL) {
        exit(1);
    }
    // no args after 'cd'
    if (arg == NULL) {
        arg = getenv("HOME");
    }
    // cd -
    if (!strcmp(arg, "-")) {
        if (!strcmp(s->lastdir->str, "")) {
            fprintf(stderr, "no previous directory\n");
            return s;
        }
        arg = s->lastdir->str;
    } else {
        // cd ~ or anything else relative to home
        if (arg[0] == '~') {
            if (arg[1] == '/' || arg[1] == '\0') {
                snprintf(path, sizeof path, "%s%s", getenv("HOME"), arg + 1);
                arg = strcat(getenv("HOME"), arg + 1);
            } else {
                // cd ~jald
                fprintf(stderr, "syntax not supported: %s\n", arg);
                return s;
            }
        }
    }
    // attempt to change current directory, returns -1 if failed
    if (chdir(arg)) {
        fprintf(stderr, "chdir: %s: %s\n", strerror(errno), path);
        return s;
    }
    // cur directory becomes last directory
    str_free(s->lastdir);
    s->lastdir = str_init(curdir);
    // set new cur directory
    str_free(s->curdir);
    s->curdir = str_init(get_cur_dir());
    return s;
}
