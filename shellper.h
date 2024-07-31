#ifndef SHELLPER_H_   /* Include guard */
#define SHELLPER_H_

enum Colours {
   BLACK = 30,
   RED = 31,
   GREEN = 32,
   YELLOW = 33,
   BLUE = 34,
   PINK = 35,
   CYAN = 36,
   WHITE = 37,
   BBLACK = 40,
   BRED = 41,
   BGREEN = 42,
   BYELLOW = 43,
   BBLUE = 44,
   BPINK = 45,
   BCYAN = 46,
   BWHITE = 47,
   NONE = 0
};

typedef struct {
    char* str;
    int len;
} String;

typedef struct {
    int count;
    char** history;
    String* curdir;
    String* lastdir;  // initialized to zero
    char** args;
} State;

char* read_line(FILE* fp);
void print_colour(char* str, int foreground, int background, char* suffix);
char** split_string(char* string, char* delim);
int array_length(char** array);
void str_free(String* string);
String* str_init(const char* string);
void free_nested_array(char** array);
int rand_col(void);
int rand_bg(void);
char* growing_ween(int size);
String* str_concat(String* start, char* end);
void shift_left(char* cmd, int cursor);
void shift_right(char* cmd, int cursor, char letter);


#endif
