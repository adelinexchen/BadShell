#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include "shellper.h"

#define READY "8=D"
#define MAX_BUFF 1000
#define MAX_PATH 1000

char* get_cur_dir(void);
State* exec_cd(char** args, State* s);

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


State* exec_cd(char** args, State* s) {
    char path[MAX_BUFF];
    if (array_length(args) > 2) {
        printf("too many arguments...");
        return s;
    }

    char* arg = args[1];
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
            printf("no previous directory\n");
            return s;
        }
        arg = s->lastdir->str;
    } else {
        /* this should be done on all words during the parse phase */
        if (*arg == '~') {
            if (arg[1] == '/' || arg[1] == '\0') {
                snprintf(path, sizeof path, "%s%s", getenv("HOME"), arg + 1);
                arg = path;
            } else {
                /* ~name should expand to the home directory of user with login `name` 
                   this can be implemented with getpwent() */
                fprintf(stderr, "syntax not supported: %s\n", arg);
                return s;
            }
        }
    }
    if (chdir(arg)) {
        fprintf(stderr, "chdir: %s: %s\n", strerror(errno), path);
        return s;
    }
    str_free(s->lastdir);
    s->lastdir = str_init(curdir);
    str_free(s->curdir);
    s->curdir = str_init(get_cur_dir());
    return s;
}

int main(int argc, char** argv) {
    State* s = (State*) malloc(sizeof(State));
    s->history = str_init("");
    s->curdir = str_init(get_cur_dir());
    s->lastdir = str_init("");
    while (true) {
        print_colour(READY, PINK, BBLUE, " ");
        // print_colour("D ", PINK, BBLUE, " ");
        
        print_colour(s->curdir->str, RED, BCYAN, " ");

        char* line = read_line(stdin); 

        char** args = split_string(line, " ");
        strcat(line, "\n");
        strcat(s->history->str, line);
        s->history = str_init(s->history->str);

        int pid = fork();

        // child
        if (!pid) {
            if (!strcmp(args[0], "cd")) {
                exec_cd(args, s);
                printf("Child: %s\n", s->lastdir->str);
                exit(0);
            } else if (!strcmp(line, "history\n")) {
                printf("%s", s->history->str);
            } else if (!strcmp(args[0], "exit")) {
                str_free(s->history);
                str_free(s->curdir);
                free(s);
                exit(0);
            }
            else {
                printf("%s", args[0]);
                execvp(args[0], args);
            }
            free_nested_array(args);
        } else {
            printf("Parent: %s\n", s->lastdir->str);
            free_nested_array(args);
            // why does nothing get executed after waitpid :(
            waitpid(pid, NULL, 0);
        }
    }
    
}
