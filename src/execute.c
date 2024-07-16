#include "psh.h"

char **PSH_TOKENIZER(char *line)
{
  size_t bufsize = 64, position = 0, i;
  char **token_arr = malloc(bufsize * sizeof(char *));
  char *token;
  int qstring = 0, has_quote = 0;
  const char *delimiters = " \t\n";
  char quote;
  if (!token_arr)
  {
    fprintf(stderr, "psh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, " ");
  while (token != NULL) // making an array of token_arr by separating the line
  {                // one by one as delim as " "
    size_t len = strlen(token);
    for(i = 0; i < strlen(token); i++)
    {       
        //printf("Token %c\n", token[i]); // DEBUGGINGi
        if ((has_quote == 0) && (token[i] == '"' || token[i] == '\''))
        {
          //printf("Accepted token: %c\n", token[i]);
          quote = token[i];
          has_quote = 1;
          break;
        }
        else if (token[i] == quote)
        {
          //printf("Accepted token: %c\n", token[i]);
          has_quote = 1;
          break;
        }
        else
        {
          has_quote = 0;
        }
    }
    // Remove quote
    if (has_quote == 1)
    {
      for (size_t j = i + 1; j < strlen(token); j++)
      {
        token[j - 1] = token[j];
      }
      token[strlen(token) - 1] = '\0';
      //printf("After quotes: %s\n", token); // DEBUGGING
      len = strlen(token);
    }
    if (qstring == 0)
    {
      token_arr[position] = malloc((len + 1) * sizeof(char));
      strcpy(token_arr[position], token);
      position++;
    }
    else
    {
      size_t prev_len = strlen(token_arr[position - 1]);
      token_arr[position - 1] = realloc(token_arr[position - 1], (prev_len + len + 2) * sizeof(char));
      if (token[len - 1] == quote)
      {
        token[len - 1] = '\0';
      }
      strcat(token_arr[position - 1], " ");
      strcat(token_arr[position - 1], token);
      //printf("Added tokens: %s\n", token_arr[position - 1]);
    }
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
    if (has_quote == 1)
    {
      qstring = !(qstring);
    }
    token = strtok(NULL, delimiters);
  }
  token_arr[position] = NULL;
  // int k = 0;
  // while(token_arr[k]!=NULL)            //debugging token_arr array
  // {
  //     printf("%s\n",token_arr[k]);
  //     k++;
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
