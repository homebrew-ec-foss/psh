// main.c
#include "psh.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Unused Parameters
int main(int argc, char **argv, char **envp) {
  printf("Welcome to psh!\n");
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

    char **token_arr = PSH_TOKENIZER(inputline);
    if (token_arr[0] != NULL) // fixed \n giving seg fault
    {
      // if (strchr(inputline,'(') && strchr(inputline, ')'))
      // {
      //   char *func_name = strtok(inputline, "(");
      //   char *func_def_start = strtok(NULL, ")");
      //   if (func_def_start != NULL)
      //   {
      //     char func_def[1024];
      //     strcpy(func_def, func_def_start);
      //     struct Func new_func;
      //     strcpy(new_func.func_name, func_name);
      //     strcpy(new_func.func_def, func_def);
      //     if(num_funcs < MAX_FUNCS)
      //     {
      //       global_funcs[num_funcs++] = new_func;
      //     }
      //     else
      //     {
      //       fprintf(stderr, "PSH: Maximum number of functions exceeded\n");
      //     }
      //   }
      //   else
      //   {
      //     fprintf(stderr, "PSH: Invalid function definition\n");
      //   }
      //   free(token_arr);
      //   continue;
      // }
      if (strchr(token_arr[0], '=')) // checks for variable assignment
      {
        char *var_name = strtok(token_arr[0], "=");
        char *var_value = strtok(NULL, "=");
        if (var_value != NULL)
        {
          if (num_vars < MAX_VARS)
          {
            //stores variables in global struct
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
      {
        run = PSH_EXEC_EXTERNAL(token_arr);
      }
      free(token_arr);
    }
  }
  free(inputline);
  return run;
}
