// #include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// #include "helper1_cd.c"
// #include "helper2_cd.c"

int PSH_READ();
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);
int PSH_EXIT(char **);
int PSH_CD(char **);
int PSH_ECHO(char **);
int PSH_PWD(char **);
int PSH_HISTORY(char **);

#define MAX_HISTORY 1024

char path_history[MAX_HISTORY][PATH_MAX];
int history_index = 0;


char *commonSuffix(char *, char *);

char* helper_cd_func1(const char* str1, const char* str2) {
    static char result[256]; // Use static to return a pointer
    char temp1[256], temp2[256];
    char *tokens1[64], *tokens2[64];
    int len1 = 0, len2 = 0;

    // Copy the input strings to temporary variables
    strcpy(temp1, str1);
    strcpy(temp2, str2);

    // Tokenize the first string by "/"
    char* token = strtok(temp1, "/");
    while (token != NULL) {
        tokens1[len1++] = token;
        token = strtok(NULL, "/");
    }

    // Tokenize the second string by "/"
    token = strtok(temp2, "/");
    while (token != NULL) {
        tokens2[len2++] = token;
        token = strtok(NULL, "/");
    }

    // Compare tokens from the end and find the point of divergence
    int i = len1 - 1;
    int j = len2 - 1;
    while (i >= 0 && j >= 0 && strcmp(tokens1[i], tokens2[j]) == 0) {
        i--;
        j--;
    }

    // Build the result string from the remaining tokens in str1
    result[0] = '\0'; // Initialize result as an empty string
    for (int k = 0; k <= i; k++) {
        strcat(result, "/");
        strcat(result, tokens1[k]);
    }

    return result;
}

// if u want to continue execution return 1 in built_in funcs

static char PREV_DIR[1024];
static char cwd[1024];
static char unresovedpath[PATH_MAX] = "";

char *builtin_str[] = {"exit", "cd", "echo", "pwd", "history"};

int (*builtin_func[])(char **) = {&PSH_EXIT, &PSH_CD, &PSH_ECHO, &PSH_PWD,
                                  &PSH_HISTORY};
// function pointer that takes in array of strings and return an int
static int is_link = 0;

int size_builtin_str =
    sizeof(builtin_str) / sizeof(builtin_str[0]); // number of built_in args

int main(int argc, char **argv, char **envp) {
  printf("Welcome to psh!\n");

  // printf("%s@", getenv("USER"));
  // for (int i=0; envp[i]!=NULL; i++) {  // accessing all env variables
  //       printf("%d: %s\n", i, envp[i]);
  //   }

  return PSH_READ();
}

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

void remove_last_component(char *path) {
  char *last_slash = strrchr(path, '/');
  if (last_slash != NULL) {
    *last_slash = '\0';
  }
}

int resolve_and_manage_symlink(char *localdir, char *resolved_path) {
    if (realpath(localdir, resolved_path) == NULL) {
        if (errno == ENOENT) {
            fprintf(stderr, "PSH: no such file or directory: %s\n", localdir);
        } else {
            perror("PSH: realpath() error");
        }
        return -1;
    }

    if (chdir(resolved_path) == -1) {
        perror("PSH: chdir() error");
        return -1;
    }

    return 0;
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
int PSH_READ() {
  size_t n = 0;
  int run = 1;
  char *inputline =
      NULL;        // NULL is required to avoind conflicts with getline function
  while (run == 1) // if not done stack smashing occurs
  {
    printf("%s@PSH %s $ ", getenv("USER"), getcwd(cwd, sizeof(cwd)));
    if (getline(&inputline, &n, stdin) == -1) {
      perror("getline");
      free(inputline);
      return -1;
    }
    inputline[strcspn(inputline, "\n")] = '\0';
    // getline takes \n as a part of string when pressed enter this.
    // line is used to remove that \n and changing it blank space

    char **token_arr = PSH_TOKENIZER(inputline);
    if (token_arr[0] != NULL) // fixed \n giving seg fault
    {
      int isinbuilt = 0;
      for (int i = 0; i < size_builtin_str; i++) {
        if (strcmp(token_arr[0], builtin_str[i]) == 0) {
          if (!strcmp(token_arr[0], "exit")) {
            run = (*builtin_func[i])(token_arr);
            free(inputline);
            exit(run);
          }
          run = (*builtin_func[i])(token_arr);
          isinbuilt = 1;
          break;
        } // frees token_arr
      }
      if (!isinbuilt)
        run = PSH_EXEC_EXTERNAL(token_arr);
      free(token_arr);
    }
  }
  free(inputline);
  return run;
}

char **PSH_TOKENIZER(char *line) {
  size_t bufsize = 64, position = 0;
  char **token_arr = malloc(bufsize * sizeof(char *));
  char *token;
  if (!token_arr) {
    fprintf(stderr, "psh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, " ");
  while (token != NULL) // making an array of token_arr by separating the line
  {                     // one by one as delim as " "
    token_arr[position] = token;
    position++;
    if (position >= bufsize) {
      bufsize += 64;
      token_arr = realloc(token_arr, bufsize * sizeof(char *));
      if (!token_arr) {
        printf("psh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, " ");
  }
  token_arr[position] = NULL; // terminating
  // int i=0;

  // while(token_arr[i]!=NULL)            //debugging token_arr array
  // {
  //     printf("%s\n",token_arr[i]);
  //     i++;
  // }
  return token_arr;
}

int PSH_EXEC_EXTERNAL(char **token_arr) {
  pid_t pid, wpid;
  int status;
  pid = fork();
  // printf("here"); //debugging
  if (pid == 0) {
    // Child process
    if (execvp(token_arr[0], token_arr) ==
        -1) // executes the binary file with args
    {
      perror("psh failed");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("psh error");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

char *commonSuffix(char *str1, char *str2) {
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int minLength = len1 < len2 ? len1 : len2;
  int i;

  // Find the length of the common suffix
  for (i = 0; i < minLength; i++) {
    if (str1[len1 - 1 - i] != str2[len2 - 1 - i]) {
      break;
    }
  }

  // Allocate memory for the result string
  char *result = (char *)malloc((i + 1) * sizeof(char));
  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  // Copy the common suffix into the result string
  strncpy(result, str1 + len1 - i, i);
  result[i] = '\0';

  return result;
}
