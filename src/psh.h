// psh.h
#ifndef PSH_H
#define PSH_H

// our functions 
#include "builtin.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <linux/limits.h>

#define MAX_HISTORY 1024
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
extern char path_history[MAX_HISTORY][PATH_MAX];
extern int history_index;
extern char cwd[1024];
extern char *builtin_str[];
extern int (*builtin_func[])(char **);
extern int size_builtin_str;
extern char PREV_DIR[1024];

extern struct Variable global_vars[MAX_VARS]; // Global array to store variables
extern int num_vars; // Number of variables currently stored
// struct Func global_funcs[MAX_FUNCS];
// int num_funcs = 0;


// Function Declarations

// main.c functions
int PSH_READ();

// execute.c functions
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);

// helper functions
void remove_last_component(char *);
int resolve_and_manage_symlink(char *, char *);
char *commonSuffix(char *, char *);
char *helper_cd_func1(const char *, const char *);
int compare_strings(const void *, const void *);
void sort_strings(char **, int);

#endif
