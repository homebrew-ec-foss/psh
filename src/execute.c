#include "psh.h"

char **PSH_TOKENIZER(char *line)
{
  size_t bufsize = 64, position = 0;
  char **token_arr = malloc(bufsize * sizeof(char *));
  char *token;
  if (!token_arr)
  {
    fprintf(stderr, "psh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, " ");
  while (token != NULL) // making an array of token_arr by separating the line
  {                     // one by one as delim as " "
    token_arr[position] = token;
    position++;
    if (position >= bufsize)
    {
      bufsize += 64;
      token_arr = realloc(token_arr, bufsize * sizeof(char *));
      if (!token_arr)
      {
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

int PSH_EXEC_EXTERNAL(char **token_arr)
{
  pid_t pid, wpid;
  int status;
  pid = fork();
  // printf("here"); //debugging
  if (pid == 0)
  {
    // Child process
    if (execvp(token_arr[0], token_arr) ==
        -1) // executes the binary file with args
    {
      fprintf(stdout, "psh: No command found: %s\n", token_arr[0]);
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Error forking
    perror("psh error");
  }
  else
  {
    // Parent process
    do
    {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1)
      {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}
