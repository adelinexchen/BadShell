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

#include <termios.h>
// #include <dos.h> // delay()

#define WEEN "   |\\      _,,,---,,_\n   /,`.-'`'    -.  ;-;;,_\n  |,4-  ) )-,_. ,\\ (  `'-'\n '---''(_/--'  `-'\\_)\n"


char* get_cur_dir(void);
State* exec_cd(State* s);
int get_prev(void);
int get_next(void);

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
    State* state = (State*) s;
    // Print value received as argument:
    if (array_length(state->args) == 0) {

    } else if (!strcmp(state->args[0], "cd")) {
        exec_cd(state);
    } else if (!strcmp(state->args[0], "history") && array_length(state->args) == 1) {
        printf("%s", state->history->str);
    } else if (!strcmp(state->args[0], "8=D")) {
        printf(WEEN);
    } else if (!strcmp(state->args[0], "exit") && array_length(state->args) == 1) {
        str_free(state->curdir);
        str_free(state->history);
        free_nested_array(state->args);
        free(s);
        exit(0);
    }
    else {
        int pid = fork();
        if (!pid) {
            execvp(state->args[0], state->args);
        } else {
            wait(NULL);
        }
    }
    free_nested_array(state->args);

    // Return reference to global variable:
    pthread_exit(s);
}

void set_non_canonical_mode(void) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ICANON; // disable canonical mode
    tty.c_lflag &= ~ECHO;   // disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void reset_terminal_mode(struct termios *saved_attributes) {
    tcsetattr(STDIN_FILENO, TCSANOW, saved_attributes);
}

int start_shell(void) {
    State* s = (State*) malloc(sizeof(State));
    s->history = str_init("");
    s->curdir = str_init(get_cur_dir());
    s->lastdir = str_init("");
    int count = 1;


    while (true) {
        char line[1000];
        memset(line, '\0', 1000);
        int n = 0;
        // print terminal prefix
        char* ready = growing_ween((count%13));
        print_colour(ready, rand_col(), rand_bg(), " ");
        print_colour(s->curdir->str, rand_col(), rand_bg(), " ");

        struct termios saved_attributes;
        tcgetattr(STDIN_FILENO, &saved_attributes);
        set_non_canonical_mode();

        char ch = '\0';

        while (ch != '\n') {
            ch = getchar();
            if (ch == '\033') { // if the first character is the escape character
                while (ch != '\n') {
                    if (ch != '\033' && ch != 127) {
                        printf("%c", ch);
                        ch = getchar();
                        continue;
                    } else if (ch == 127) {
                        printf("\b\033[K");
                        line[n-1] = '\0';
                        n -= 1;
                    }
                getchar(); // skip the '['
                switch(getchar()) { // the actual arrow key
                    case 'A':
                        // up arrow
                        printf("\33[2K\r");
                        print_colour(ready, rand_col(), rand_bg(), " ");
                        print_colour(s->curdir->str, rand_col(), rand_bg(), " ");
                        printf("up key");
                        break;
                    case 'B':
                        printf("\33[2K\r");
                        print_colour(ready, rand_col(), rand_bg(), " ");
                        print_colour(s->curdir->str, rand_col(), rand_bg(), " ");
                        printf("down key");
                        break;
                    case 'C':
                        // right arrow
                        // printf("\033[1C");
                        break;
                    case 'D':
                        // left arrow
                        // printf("\b");
                        break;
                    default:
                        // other
                        printf("wtf\n");
                        break;
                }
                ch = getchar();
                }
                printf("\n");
                line[n] = ch;
            } else if (ch == 127) {
                // back space
                if (n > 0) {
                    // move back and delete char
                    printf("\b\033[K");
                    // printf("\n%d, %s\n", n-1, line);
                    line[n-1] = '\0';
                    n -= 1;
                }
                // printf("\nbackspace: %d, %s\n", n, line);
            } else {
                putchar(ch);
                line[n] = ch;
                n = n + 1;
            }
            // printf(" %d\n", n);
        }
        reset_terminal_mode(&saved_attributes);
        printf("%s\n", line);
        line[n-1] = '\0';
        s->args = split_string(line, " ");
        // start to check and execute command
        pthread_t id;
        pthread_create(&id, NULL, foo, s);
        // Wait for foo() and retrieve value in ptr;
        pthread_join(id, (void**)s);

        // append command to history
        s->history = str_concat(s->history, line);
        s->history = str_concat(s->history, "\n");
        
        free(ready);
        count = count + 1;
    }
}

int main(int argc, char** argv) {
    return start_shell();
}
