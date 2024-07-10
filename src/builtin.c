// builtin.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "psh.h"
#include "builtin.h"


char path_history[MAX_HISTORY][PATH_MAX];
int history_index = 0;
char cwd[1024];
char *builtin_str[] = {"exit", "cd", "echo", "pwd", "history"};
int (*builtin_func[])(char **) = {&PSH_EXIT, &PSH_CD, &PSH_ECHO, &PSH_PWD, &PSH_HISTORY};
int size_builtin_str = sizeof(builtin_str) / sizeof(builtin_str[0]);

int PSH_EXIT(char **token_arr) {
  if (!token_arr[1]) {
    printf("bye bye PSH :D\n"); // handling empty args and freeing token array
                                // before leaving
    free(token_arr);
    return 0;
    // exit(0);
  }
  printf("bye bye PSH :D\n");
  int exit_code = atoi(token_arr[1]);
  free(token_arr);
  return exit_code;
  // exit(exit_code);char
}


int PSH_CD(char **token_arr) {
    char current_dir[PATH_MAX];
    char *localdir = malloc(PATH_MAX);
    char *home = NULL; // for ~ and empty cases
    static char PREV_DIR[PATH_MAX] = "";      // for - cases

    if (localdir == NULL) {
        perror("PSH: malloc() error");
        return 1;
    }

    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("PSH: getcwd() error");
        free(localdir);
        return 1;
    }

    if (token_arr[1] == NULL || strcmp(token_arr[1], "~") == 0) {
        home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "PSH: HOME environment variable not set\n");
            free(localdir);
            return 1;
        }
        strncpy(localdir, home, PATH_MAX - 1);
        localdir[PATH_MAX - 1] = '\0';
    } else if (strcmp(token_arr[1], "-") == 0) {
        if (PREV_DIR[0] == '\0') {
            fprintf(stderr, "PSH: No previous directory\n");
            free(localdir);
            return 1;
        }
        strncpy(localdir, PREV_DIR, PATH_MAX - 1);
        localdir[PATH_MAX - 1] = '\0';
    } else if (strcmp(token_arr[1], "..") == 0) {
        if (history_index > 0) {
            history_index--;
            strncpy(localdir, path_history[history_index], PATH_MAX - 1);
            localdir[PATH_MAX - 1] = '\0';
        } else {
            snprintf(localdir, PATH_MAX, "%s/..", current_dir);
        }
    } else {
        if (token_arr[1][0] != '/') {
            snprintf(localdir, PATH_MAX, "%s/%s", current_dir, token_arr[1]);
        } else {
            strncpy(localdir, token_arr[1], PATH_MAX - 1);
            localdir[PATH_MAX - 1] = '\0';
        }
    }

    char resolved_path[PATH_MAX];
    if (resolve_and_manage_symlink(localdir, resolved_path) == -1) {
        free(localdir);
        return 1;
    }

    // Save the logical path in the history
    if (token_arr[1]!=NULL && strcmp(token_arr[1], "..") != 0 && strcmp(token_arr[1], "-") != 0) {
        strncpy(path_history[history_index], resolved_path, PATH_MAX - 1);
        path_history[history_index][PATH_MAX - 1] = '\0';
        history_index++;
        // printf("%s\n",*path_history);
    }

    if (strcmp(localdir, resolved_path) != 0) {
        printf("PSH: followed symlink: %s -> %s\n", localdir, resolved_path);
    }

    strncpy(PREV_DIR, current_dir, PATH_MAX - 1);
    PREV_DIR[PATH_MAX - 1] = '\0';

    free(localdir);
    return 1;
}


int PSH_ECHO(char **token_arr) { //
                                 // Sid
                                 //   int i = 1;
                                 //   printf("%s",token_arr[0]); // debugging
                                 //   // while (token_arr[i] != "\n") {
                                 //   // printf("%s",token_arr[i]);
                                 //   // }
  return 1;
}


int PSH_PWD(char **token_arr) {

  // printf("%s\n", token_arr[1]); //debugging
  // printf("Printing current working directory\n"); //debugging

  char rpath[1024];
  // printf("%d\n", PATH_MAX);

  if (token_arr[1] == NULL ||
      strcmp(token_arr[1], "-L") == 0) // Default pwd and pwd -L
  {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("%s\n", cwd);
    } else {
      perror("PSH: getcwd() error");
    }
  } else if ((strcmp(token_arr[1], "-P")) == 0) {
    if (realpath(cwd, rpath) != NULL) // pwd -P
    {
      printf("%s\n", rpath);
    } else {
      perror("PSH: realpath() error");
    }
  } else {
    fprintf(stderr, "Unknown option: %s\n", token_arr[1]);
  }
  return 1;
}


int PSH_HISTORY(char **token_arr) {
  // your code goes here Alayna
  return 1;
}
