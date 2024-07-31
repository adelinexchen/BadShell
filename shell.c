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

#define WEEN "   |\\      _,,,---,,_\n   /,`.-'`'    -.  ;-;;,_\n  |,4-  ) "\
    ")-,_. ,\\ (  `'-'\n '---''(_/--'  `-'\\_)\n"
#define MAX_BUFF 1000

int get_prev(void);
int get_next(void);

void* run(void* s){
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

void arrow_key_press(char ch) {
    
}

int start_shell(void) {
    // state of the shell
    State* s = (State*) malloc(sizeof(State));
    // history of commands
    s->history = str_init("");
    // cwd
    s->curdir = str_init(get_cur_dir());
    // last accessed directory if there is one
    s->lastdir = str_init("");
    int count = 1;

    // allows arrow keys to be detected
    struct termios saved_attributes;
    tcgetattr(STDIN_FILENO, &saved_attributes);
    set_non_canonical_mode();

    while (true) {
        // where the cursor is on the terminal
        int cursor = 0;
        // how long the command is
        int cmd_len = 0;
        char line[MAX_BUFF];
        // clear the array
        memset(line, '\0', MAX_BUFF);
        // print terminal prefix
        char* ready = growing_ween((count%13));
        print_colour(ready, rand_col(), rand_bg(), " ");
        print_colour(s->curdir->str, rand_col(), rand_bg(), " ");

        char ch = getchar();

        while (ch != '\n') {
            if (cmd_len >= MAX_BUFF) {
                exit(1);
            }
            if (ch == '\033') { // if the first character is the escape character
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
                        if (cursor < cmd_len) {
                            printf("\033[1C");
                            cursor += 1;
                        }
                        break;
                    case 'D':
                        // left arrow
                        if (cursor > 0) {
                            printf("\033[D");
                            cursor -= 1;
                        }
                        break;
                    default:
                        // other
                        printf("wtf\n");
                        break;
                }
            } else if (ch == 127) {
                // back space
                if (cursor > 0) {
                    // delete character from under cursor
                    shift_left(line, cursor-1);
                    // reflect this change in stdout 
                    // delete till end of line (stdout)
                    printf("\b\033[K");
                    // save cursor pos
                    printf("\033[s");
                    cursor -= 1;
                    cmd_len -= 1;
                    // reprint to stdout and move cursor back
                    printf("%s", line+cursor);
                    printf("\033[u");
                }
            } else {
                // insert character in under cursor
                shift_right(line, cursor, ch);
                // reflect this change in stdout 
                // print added char
                putchar(ch);
                // save cursor position
                printf("\033[s");
                cursor += 1;
                cmd_len += 1;
                // print everything after cursor and update cursor pos
                printf("%s", line+cursor);
                printf("\033[u");
            }
            ch = getchar();
        }
        printf("\ncmd: %s\n", line);
        printf("\n");
        // get user's command and args
        s->args = split_string(line, " ");
        // start to check and execute command
        pthread_t id;
        pthread_create(&id, NULL, run, s);
        // Wait for run() and retrieve value in ptr;
        pthread_join(id, (void**)s);

        // append command to history
        s->history = str_concat(s->history, line);
        s->history = str_concat(s->history, "\n");
        
        free(ready);
        count = count + 1;
    }
    reset_terminal_mode(&saved_attributes);
    // free everything
}

int main(int argc, char** argv) {
    // printf("ac\033[Da\n");
    return start_shell();
    // char l[1000] = "1235";
    // printf("%s\n", l);
    // shift_right(l, 3, '4');
    // printf("%s\n", l);
    return 1;
}
