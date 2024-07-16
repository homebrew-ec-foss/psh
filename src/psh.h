// psh.h
#ifndef PSH_H
#define PSH_H

// Our Headers
#include "builtin.h"

// C Headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_HISTORY 1024
#define MAX_LINE_LENGTH 1024
#define MAX_VARS 100
// #define MAX_FUNCS 100

// Defining Structs to hold variables and functions
struct Variable
{
    char var_name[64];
    char var_value[256];
};
// struct Func              // function declarations
// {
//     char func_name[64];
//     char func_def[1024];
// };

// Global Variables
extern char cwd[PATH_MAX];
extern char *builtin_str[];
extern int (*builtin_func[])(char **);
extern int size_builtin_str;
extern char PREV_DIR[PATH_MAX];
extern char PATH[PATH_MAX];


extern struct Variable global_vars[MAX_VARS]; // Global array to store variables
extern int num_vars; // Number of variables currently stored
// struct Func global_funcs[MAX_FUNCS];
// int num_funcs = 0;


// Function Declarations

// main.c functions
int PSH_READ(void);

// execute.c functions
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);

// helper functions
void remove_last_component(char *);
void get_last_path_component(const char *, char *);
void delete_file(const char *);
void read_lines(const char *, int, int);
void read_lines_wo_no(const char *, int, int);
int count_lines(const char *);
void read_lines_reverse(const char *, int, int);
void remove_line(const char *, size_t);
void clear_session_history(void);
void read_lines_reverse_wo_no(const char *, int, int);
char *expand_history(const char *, FILE *);
int compare_strings(const void *, const void *);
void sort_strings(char **, int);
char **split_commands(char *input);
#endif
