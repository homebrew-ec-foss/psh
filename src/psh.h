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
#define MAX_LINE_LENGTH 1024
#define SESSION_HISTORY_FILE "session_history_file.txt"
#define MEMORY_HISTORY_FILE "history_file.txt"

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
void delete_file(const char *);
void read_lines(const char *, int, int);
void read_lines_wo_no(const char *, int, int);
int count_lines(const char *); 
void read_lines_reverse(const char *, int, int);
void remove_line(const char *, size_t);
void clear_session_history();
char *expand_history(const char *, FILE *);

#endif
