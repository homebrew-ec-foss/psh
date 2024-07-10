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

// Global Variables
extern char path_history[MAX_HISTORY][PATH_MAX];
extern int history_index;
extern char cwd[1024];
extern char *builtin_str[];
extern int (*builtin_func[])(char **);
extern int size_builtin_str;
extern char PREV_DIR[1024];

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

#endif
