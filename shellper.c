#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include "shellper.h"

/* String* str_init(const char* string)
 *
 * Creates a new String* struct pointer
 *
 * string: the string being initialised
 *
 * returns: a pointer to a String struct
 */
String* str_init(const char* string) {
    String* s = malloc(sizeof(String));
    s->len = strlen(string);
    s->str = malloc(s->len + 1);
    strcpy(s->str, string);
    return s;
}

/* void str_free(String* string)
 * 
 * free String* struct
 * 
 * string: String* to be freed
 */
void str_free(String* string) {
    free(string->str);
    free(string);
}

/* void free_nested_array(char** array, int len)
 * 
 * Frees a nested array of chars
 *
 * array: array of strings
 * len: length of array
 *
 * return: void
 */
void free_nested_array(char** array) { 
    for (int i = 0; i < array_length(array); i++) {
        free(array[i]);
    }
    free(array);
} 

/* int array_length(char** array) 
 *   
 * Finds the length of an array
 *
 * array: the array of strings you want the size of
 * 
 * returns: size of array
*/
int array_length(char** array) {
    int len = 0;
    for (int i = 0; array[i]; i++) {
        len++;
    }
    return len;
}

/* char* read_line(FILE* fp)
   Reads a line from stdin provided by user
   returns: the line read without the newline character at the end
*/
char* read_line(FILE* fp) {
    char* line = NULL;
    size_t len = 0;
    getline(&line, &len, fp);
    // strcspn finds the index of the first occurance of "\n" string
    // we then replace it with the null character
    line[strcspn(line, "\n")] = '\0';
    return line;
}

/*
+---------+------------+------------+
|  color  | foreground | background |
|         |    code    |    code    |
+---------+------------+------------+
| black   |     30     |     40     |
| red     |     31     |     41     |
| green   |     32     |     42     |
| yellow  |     33     |     43     |
| blue    |     34     |     44     |
| magenta |     35     |     45     |
| cyan    |     36     |     46     |
| white   |     37     |     47     |
+---------+------------+------------+
*/
void print_colour(char* str, int foreground, int background, char* suffix) {
    if (background != 0 && foreground != 0) {
        printf("\x1b[%d;%dm %s\x1b[0m", foreground, background, str);
    } else if (background != 0) {
        printf("\x1b[%dm %s\x1b[0m", background, str);
    } else if (foreground != 0) {
        printf("\x1b[%dm %s\x1b[0m", foreground, str);
    } else {
        printf("%s", str);
    }
    if (suffix != NULL) {
        printf("%s", suffix);
    }
}

char** split_string(char* string, char* delim) {
    char** split = malloc(sizeof(char*) * 1);
    char* token = strtok(string, delim);
    // loop through the string to extract all other tokens
    int i = 0;
    while( token != NULL ) {
        split[i] = strdup(token);
        token = strtok(NULL, delim);
        i += 1;
        split = realloc(split, (i+1) * sizeof(char*));
    }
    split[i] = NULL;
    return split;
}

int rand_col(void) {
    int col = (rand() % 8) + 30;
    return col;
}

int rand_bg(void) {
    int col = (rand() % 8) + 40;
    return col;
}

char* growing_ween(int size) {
    char* ready = malloc(size + 1 + 2);
    strcat(ready, "8");

    for (int i = 0; i < size; i++) {
        strcat(ready, "=");
    }

    strcat(ready, "D");
    return ready;
}

/* String* str_concat(String* start, char* end)
 *
 * given two strings, concatinates them together
 *
 * start: the start string as a String struct
 * end: the end string
 *
 * returns: a concatenated string
 */
String* str_concat(String* start, char* end) {
    char* str = malloc(start->len + strlen(end) + 1);
    strcpy(str, start->str);
    strcat(str, end);
    str_free(start);
    String* fin = str_init(str);
    free(str);
    return fin;
}
