#include "psh.h"
char path_history[MAX_HISTORY][PATH_MAX];
int history_index = 0;
char cwd[1024];
char *builtin_str[] = {"exit", "cd", "echo", "pwd", "history", "export"};
int (*builtin_func[])(char **) = {&PSH_EXIT, &PSH_CD, &PSH_ECHO, &PSH_PWD, &PSH_HISTORY, &PSH_EXPORT};
int size_builtin_str = sizeof(builtin_str) / sizeof(builtin_str[0]);
struct Variable global_vars[MAX_VARS];
int num_vars = 0;

int PSH_EXIT(char **token_arr)
{
  if (!token_arr[1])
  {
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

int PSH_CD(char **token_arr)
{
  char current_dir[PATH_MAX];
  char *localdir = malloc(PATH_MAX);
  char *home = NULL;                   // for ~ and empty cases
  static char PREV_DIR[PATH_MAX] = ""; // for - cases

  if (localdir == NULL)
  {
    perror("PSH: malloc() error");
    return 1;
  }

  if (getcwd(current_dir, sizeof(current_dir)) == NULL)
  {
    perror("PSH: getcwd() error");
    free(localdir);
    return 1;
  }

  if (token_arr[1] == NULL || strcmp(token_arr[1], "~") == 0)
  {
    home = getenv("HOME");
    if (home == NULL)
    {
      fprintf(stderr, "PSH: HOME environment variable not set\n");
      free(localdir);
      return 1;
    }
    strncpy(localdir, home, PATH_MAX - 1);
    localdir[PATH_MAX - 1] = '\0';
  }
  else if (strcmp(token_arr[1], "-") == 0)
  {
    if (PREV_DIR[0] == '\0')
    {
      fprintf(stderr, "PSH: No previous directory\n");
      free(localdir);
      return 1;
    }
    strncpy(localdir, PREV_DIR, PATH_MAX - 1);
    localdir[PATH_MAX - 1] = '\0';
  }
  else if (strcmp(token_arr[1], "..") == 0)
  {
    if (history_index > 0)
    {
      history_index--;
      strncpy(localdir, path_history[history_index], PATH_MAX - 1);
      localdir[PATH_MAX - 1] = '\0';
    }
    else
    {
      snprintf(localdir, PATH_MAX, "%s/..", current_dir);
    }
  }
  else
  {
    if (token_arr[1][0] != '/')
    {
      snprintf(localdir, PATH_MAX, "%s/%s", current_dir, token_arr[1]);
    }
    else
    {
      strncpy(localdir, token_arr[1], PATH_MAX - 1);
      localdir[PATH_MAX - 1] = '\0';
    }
  }

  char resolved_path[PATH_MAX];
  if (resolve_and_manage_symlink(localdir, resolved_path) == -1)
  {
    free(localdir);
    return 1;
  }

  // Save the logical path in the history
  if (token_arr[1] != NULL && strcmp(token_arr[1], "..") != 0 && strcmp(token_arr[1], "-") != 0)
  {
    strncpy(path_history[history_index], resolved_path, PATH_MAX - 1);
    path_history[history_index][PATH_MAX - 1] = '\0';
    history_index++;
    // printf("%s\n",*path_history);
  }

  if (strcmp(localdir, resolved_path) != 0)
  {
    printf("PSH: followed symlink: %s -> %s\n", localdir, resolved_path);
  }

  strncpy(PREV_DIR, current_dir, PATH_MAX - 1);
  PREV_DIR[PATH_MAX - 1] = '\0';

  free(localdir);
  return 1;
}

int PSH_ECHO(char **token_arr)
{ //
  // Sid
  //   int i = 1;
  //   printf("%s",token_arr[0]); // debugging
  //   // while (token_arr[i] != "\n") {
  //   // printf("%s",token_arr[i]);
  //   // }
  return 1;
}

int PSH_PWD(char **token_arr)
{

  // printf("%s\n", token_arr[1]); //debugging
  // printf("Printing current working directory\n"); //debugging

  char rpath[1024];
  // printf("%d\n", PATH_MAX);

  if (token_arr[1] == NULL ||
      strcmp(token_arr[1], "-L") == 0) // Default pwd and pwd -L
  {
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      printf("%s\n", cwd);
    }
    else
    {
      perror("PSH: getcwd() error");
    }
  }
  else if ((strcmp(token_arr[1], "-P")) == 0)
  {
    if (realpath(cwd, rpath) != NULL) // pwd -P
    {
      printf("%s\n", rpath);
    }
    else
    {
      perror("PSH: realpath() error");
    }
  }
  else
  {
    fprintf(stderr, "Unknown option: %s\n", token_arr[1]);
  }
  return 1;
}

int PSH_HISTORY(char **token_arr)
{
  // your code goes here Alayna
  return 1;
}

int PSH_EXPORT(char **token_arr) 
{
    if (token_arr[1] == NULL || strcmp(token_arr[1], "-p") == 0) // default null or -p option
    {
        extern char **environ;

        // find no of entries in environ
        int env_length = 0;
        while (environ[env_length] != NULL) 
        {
            env_length++;
        }

        sort_strings(environ, env_length); // sort environment variables

        int i = 0;
        while (environ[i] != NULL) 
        {
            printf("declare -x %s\n", environ[i]); //print all env variables
            i++;
        }
    } 
    else if (strcmp(token_arr[1], "-f") == 0) // functions
    {
        // int found = 0;
        // for (int i = 0; i < num_funcs; i++) 
        // {
        //     printf("%s\n%s\n", global_funcs[i].func_name, global_funcs[i].func_def);
        //     if (strcmp(token_arr[2], global_funcs[i].func_name) == 0) 
        //     {
        //         if (setenv(global_funcs[i].func_name, global_funcs[i].func_def, 1) != 0) 
        //         {
        //             perror("PSH: setenv() error");
        //         }
        //         found = 1;
        //         break;
        //     }
        // }

        // if (!found) 
        // {
        //     if (strchr(token_arr[2], '{')) 
        //     {
        //         char *func_name = strtok(token_arr[2], "{");
        //         char *func_value_start = strchr(token_arr[2], '{');

        //         if (func_value_start != NULL) 
        //         {
        //             char *func_value_end = strrchr(token_arr[2], '}');
        //             if (func_value_end != NULL) 
        //             {
        //                 size_t func_value_length = func_value_end - func_value_start + 1;
        //                 char *func_value = malloc(func_value_length);
        //                 strncpy(func_value, func_value_start, func_value_length - 1);
        //                 func_value[func_value_length - 1] = '\0';

        //                 // Set the environment variable
        //                 if (setenv(func_name, func_value, 1) != 0) 
        //                 {
        //                     perror("PSH: setenv() error");
        //                 }

        //                 free(func_value);
        //             } 
        //             else 
        //             {
        //                 fprintf(stderr, "PSH: Invalid function definition\n");
        //             }
        //         } 
        //         else 
        //         {
        //             fprintf(stderr, "PSH: Invalid function definition\n");
        //         }
        //     } 
        //     else 
        //     {
        //         fprintf(stderr, "PSH: Invalid function definition\n");
        //     }
        // }
    } 
    else if (strcmp(token_arr[1], "-n") == 0) // removing and env variable
    {
        if (token_arr[2] != NULL) 
        {
            if (unsetenv(token_arr[2]) != 0) 
            {
                perror("PSH: unsetenv() error");
            }
        }
    } 
    else 
    {
        // setting a variable as an env variable
        int found = 0;
        for (int i = 0; i < num_vars; i++) 
        {
            if (strcmp(token_arr[1], global_vars[i].var_name) == 0) 
            {
                if (setenv(global_vars[i].var_name, global_vars[i].var_value, 1) != 0) 
                {
                    perror("PSH: setenv() error");
                }
                found = 1;
                break;
            }
        }

        if (!found) 
        {
            if (strchr(token_arr[1], '=')) 
            {
                char *var_name = strtok(token_arr[1], "=");
                char *var_value = strtok(NULL, "=");

                if (var_value != NULL) 
                {
                    // Set the environment variable
                    if (setenv(var_name, var_value, 1) != 0) 
                    {
                        perror("PSH: setenv() error");
                    }
                } 
                else 
                {
                    fprintf(stderr, "PSH: Invalid variable assignment\n");
                }
            } 
            else 
            {
                fprintf(stderr, "Unknown option: %s\n", token_arr[1]);
            }
        }
    }

    return 1;
}

