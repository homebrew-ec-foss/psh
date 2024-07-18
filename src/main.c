// main.c
#include "psh.h"
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

  getcwd(cwd, sizeof(cwd)); // home/$USER/psh
  strcpy(PATH,cwd);
  return PSH_READ();
}

int PSH_READ(void) {
  size_t n = 0;
  int run = 1;
  char *inputline = NULL;        // NULL is required to avoind conflicts with getline function


  while (run == 1) // if not done stack smashing occurs
  {
    char last_component[PATH_MAX];
    get_last_path_component(PATH, last_component);

    if(strcmp(PATH, "/") == 0)
      printf("%s@PSH → %s $ ", getenv("USER"), "/");

    else
      printf("%s@PSH → %s $ ", getenv("USER"), last_component);

    if (getline(&inputline, &n, stdin) == -1)
    {
      perror("getline");

      free(inputline);
      return -1;
    }
    inputline[strcspn(inputline, "\n")] = '\0';
    // getline takes \n as a part of string when pressed enter this.
    // line is used to remove that \n and changing it blank space

     // Ignore comments starting with '#'
        char *comment_pos = strchr(inputline, '#');
        if (comment_pos) {
            *comment_pos = '\0';
        }

        // Skip processing if the line becomes empty after stripping comments
        if (inputline[0] == '\0') {
            continue;
        }


    // Writing the commands to the global and session history file
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


    if (fp1 == NULL && fp2 == NULL) {
      perror("Error:");
      return 1;
    } else {
      fprintf(fp1, "%s\n", inputline);
      fprintf(fp2, "%s\n", inputline);
    }
    fclose(fp1);
    fclose(fp2);

        char **commands = split_commands(inputline);
        for (int i = 0; commands[i] != NULL; i++)
        {
            char **token_arr = PSH_TOKENIZER(commands[i]);
            if (token_arr[0] != NULL)
            {
                if (strchr(token_arr[0], '='))
                {
                    char *var_name = strtok(token_arr[0], "=");
                    char *var_value = strtok(NULL, "=");
                    if (var_value != NULL)
                    {
                        if (num_vars < MAX_VARS)
                        {
                            strcpy(global_vars[num_vars].var_name, var_name);
                            strcpy(global_vars[num_vars].var_value, var_value);
                            num_vars++;
                        }
                        else
                        {
                            fprintf(stderr, "PSH: Invalid variable assignment\n");
                        }
                        free(token_arr);
                        continue;
                    }
                }
                int isinbuilt = 0;
                for (int j = 0; j < size_builtin_str; j++)
                {
                    if (strcmp(token_arr[0], builtin_str[j]) == 0)
                    {
                        if (!strcmp(token_arr[0], "exit"))
                        {
                            run = (*builtin_func[j])(token_arr);
                            free(inputline);
                            exit(run);
                        }
                        run = (*builtin_func[j])(token_arr);
                        isinbuilt = 1;
                        break;
                    }
                }
                if (!isinbuilt)
                {
                    run = PSH_EXEC_EXTERNAL(token_arr);
                }
                free(token_arr);
            }
        }
        free(commands);
    }
    free(inputline);
    return run;
}


int PSH_SCRIPT(const char *file) {

  FILE *script = fopen(file, "r");
  if (script == NULL) {
    fprintf(stderr, "FILE open failed");
    return -1;
  }

  else {
    char **script_lines = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_count = 0;

    // Read lines and store them in the array
    while ((read = getline(&line, &len, script)) != -1) {
      script_lines = realloc(script_lines, sizeof(char *) * (line_count + 1));
      if (script_lines == NULL) {
        fprintf(stderr, "Memory allocation failed");
        fclose(script);
        free(line);
        return -1;
      }

      // Remove newline character if present
      if (line[read - 1] == '\n') {
        line[read - 1] = '\0';
      }

      script_lines[line_count] = strdup(line);
      line_count++;
    }

    for (int i = 0; script_lines[i] != NULL; i++) { // debugging
      //   printf("%s\n",script_lines[i]);
      system(script_lines[i]);
    }

    free(line);
    fclose(script);

    for (int i = 0; i < line_count; i++) {
      free(script_lines[i]);
    }
    free(script_lines);
  }
  return 1;
}