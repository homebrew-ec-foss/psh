// main.c
#include "psh.h"

int main(int argc, char **argv, char **envp)
{
  printf("Welcome to psh!\n");

  // printf("%s@", getenv("USER"));
  // for (int i=0; envp[i]!=NULL; i++) {  // accessing all env variables
  //       printf("%d: %s\n", i, envp[i]);
  //   }

  return PSH_READ();
}

int PSH_READ()
{
  size_t n = 0;
  int run = 1;
  char *inputline =
      NULL;        // NULL is required to avoind conflicts with getline function
  while (run == 1) // if not done stack smashing occurs
  {
    printf("%s@PSH %s $ ", getenv("USER"), getcwd(cwd, sizeof(cwd)));
    if (getline(&inputline, &n, stdin) == -1)
    {
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
      for (int i = 0; i < size_builtin_str; i++)
      {
        if (strcmp(token_arr[0], builtin_str[i]) == 0)
        {
          if (!strcmp(token_arr[0], "exit"))
          {
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
