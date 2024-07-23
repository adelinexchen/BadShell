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
#include <pthread.h>

#define READY "8=D"
#define MAX_BUFF 1000
#define MAX_PATH 1000

char* get_cur_dir(void);
State* exec_cd(State* s);

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

void* foo(void* s){
    // Print value received as argument:
    if (!strcmp(((State*) s)->args[0], "cd")) {
        exec_cd(((State*) s));
        printf("Child: %s\n", ((State*) s)->lastdir->str);

    } else if (!strcmp(((State*) s)->args[0], "history\n")) {
        printf("%s", ((State*) s)->history->str);

    } else if (!strcmp(((State*) s)->args[0], "exit")) {
        str_free(((State*) s)->history);
        str_free(((State*) s)->curdir);
        free_nested_array(((State*) s)->args);
        free(s);
        exit(0);
    }
    else {
        int pid = fork();
        if (!pid) {
            execvp(((State*) s)->args[0], ((State*) s)->args);
        } else {
            wait(NULL);
        }
    }
    free_nested_array(((State*) s)->args);

    // Return reference to global variable:
    pthread_exit(s);
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

        s->args = split_string(line, " ");
        strcat(line, "\n");
        strcat(s->history->str, line);
        s->history = str_init(s->history->str);

        pthread_t id;
        pthread_create(&id, NULL, foo, s);
        // Wait for foo() and retrieve value in ptr;
        pthread_join(id, (void**)s);

    }
    
}
