// main.c
#include "psh.h"
// #include <cerrno>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Unused Parameters
int main(int argc, char **argv, char **envp) {
  printf("Welcome to psh!\n");

  if(argc == 2)
  {
      return PSH_SCRIPT(argv[1]);
  }

  // printf("%s@", getenv("USER"));
  // for (int i=0; envp[i]!=NULL; i++) {  // accessing all env variables
  //       printf("%d: %s\n", i, envp[i]);
  //   }
  else {
  getcwd(cwd, sizeof(cwd)); // home/$USER/psh
  strcpy(PATH,cwd);
  return PSH_READ();
  }
}

int PSH_READ(void) {
    size_t n = 0;
    int run = 1;
    char *inputline = NULL;

    while (run == 1) {
        char last_component[PATH_MAX];
        get_last_path_component(PATH, last_component);

        if (strcmp(PATH, "/") == 0)
            printf("%s@PSH → %s $ ", getenv("USER"), "/");
        else
            printf("%s@PSH → %s $ ", getenv("USER"), last_component);

        if (getline(&inputline, &n, stdin) == -1) {
            perror("getline");
            free(inputline);
            return -1;
        }
        inputline[strcspn(inputline, "\n")] = '\0';

        char *comment_pos = strchr(inputline, '#');
        if (comment_pos) {
            *comment_pos = '\0';
        }

        if (inputline[0] == '\0') {
            continue;
        }

        FILE *fp1;
        FILE *fp2;

        char path_memory[PATH_MAX];
        strcpy(path_memory, cwd);
        strcat(path_memory, "/.files/MEMORY_HISTORY_FILE");
        fp1 = fopen(path_memory, "a");

        char path_session[PATH_MAX];
        strcpy(path_session, cwd);
        strcat(path_session, "/.files/SESSION_HISTORY_FILE");
        fp2 = fopen(path_session, "a");

        if (fp1 == NULL || fp2 == NULL) {
            perror("Error:");
            if (fp1) fclose(fp1);
            if (fp2) fclose(fp2);
            free(inputline);
            return 1;
        } else {
            fprintf(fp1, "%s\n", inputline);
            fprintf(fp2, "%s\n", inputline);
            fclose(fp1);
            fclose(fp2);
        }

        char **commands = split_commands(inputline);
        for (int i = 0; commands[i] != NULL; i++) {
            char **token_arr = PSH_TOKENIZER(commands[i]);
            if (token_arr[0] != NULL) {
                if (strchr(token_arr[0], '=')) {
                    char *var_name = strtok(token_arr[0], "=");
                    char *var_value = strtok(NULL, "=");
                    if (var_value != NULL) {
                        if (num_vars < MAX_VARS) {
                            strcpy(global_vars[num_vars].var_name, var_name);
                            strcpy(global_vars[num_vars].var_value, var_value);
                            num_vars++;
                        } else {
                            fprintf(stderr, "PSH: Invalid variable assignment\n");
                        }
                        free_double_pointer(token_arr);
                        continue;
                    }
                }
                int isinbuilt = 0;
                for (int j = 0; j < size_builtin_str; j++) {
                    if (strcmp(token_arr[0], builtin_str[j]) == 0) {
                        if (!strcmp(token_arr[0], "exit")) {
                            run = (*builtin_func[j])(token_arr);
                            free(inputline);
                            free_double_pointer(commands);
                            return run;
                        }
                        run = (*builtin_func[j])(token_arr);
                        isinbuilt = 1;
                        break;
                    }
                }
                if (!isinbuilt) {
                    run = PSH_EXEC_EXTERNAL(token_arr);
                }
                free_double_pointer(token_arr);
            }
        }
        free_double_pointer(commands);
    }
    free(inputline);
    return run;
}

int PSH_SCRIPT(const char *file) {

  FILE *script = fopen(file, "r");

  int run = 1;
  size_t n = 0;
  char *inputline = NULL;
  int result = 0;
  
  if (script == NULL) {
    fprintf(stderr, "FILE open failed\n");
    run = 0;
    return -1;
  }


  while (run == 1) {
    if (getline(&inputline, &n, script) == -1) {
      if (feof(script)) {
        // Reached end of file
        // feof() doesn't actually detect the end of the file itself.
        // Instead, it reports whether a previous read operation has attempted
        // to read past the end of the file.
        break;
      }
      if (errno != 0) {
        perror("getline");
        result = -1;
      }
      break;
    }

    inputline[strcspn(inputline, "\n")] = '\0';

    // Ignore comments starting with '#'
    char *comment_pos = strchr(inputline, '#');
    if (comment_pos) {
      *comment_pos = '\0';
    }

    // Skip processing if the line becomes empty after stripping comments
    if (inputline[0] == '\0') {
      continue;
    }

    char **commands = split_commands(inputline);
    for (int i = 0; commands[i] != NULL; i++) {
      char **token_arr = PSH_TOKENIZER(commands[i]);
      if (token_arr[0] != NULL) {
        if (strchr(token_arr[0], '=')) {
          char *var_name = strtok(token_arr[0], "=");
          char *var_value = strtok(NULL, "=");
          if (var_value != NULL) {
            if (num_vars < MAX_VARS) {
              strcpy(global_vars[num_vars].var_name, var_name);
              strcpy(global_vars[num_vars].var_value, var_value);
              num_vars++;
            } else {
              fprintf(stderr, "PSH: Invalid variable assignment\n");
            }
            free_double_pointer(token_arr);
            continue;
          }
        }
        int isinbuilt = 0;
        for (int j = 0; j < size_builtin_str; j++) {
          if (strcmp(token_arr[0], builtin_str[j]) == 0) {
            if (!strcmp(token_arr[0], "exit")) {
              run = (*builtin_func[j])(token_arr);
              free_double_pointer(token_arr);
              free_double_pointer(commands);
              free(inputline);
              fclose(script);
              return run;
            }
            run = (*builtin_func[j])(token_arr);
            isinbuilt = 1;
            break;
          }
        }
        if (!isinbuilt) {
          run = PSH_EXEC_EXTERNAL(token_arr);
        }
        free_double_pointer(token_arr);
      }
    }
    free_double_pointer(commands);
  }

  free(inputline);
  fclose(script);
  return result;
}
  