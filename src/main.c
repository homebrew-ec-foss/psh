#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int PSH_READ();
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);
int PSH_EXIT(char **);

char *builtin_str[] = {"exit"};

int (*builtin_func[])(char **) = {&PSH_EXIT}; //function pointer that takes in array of strings and return an int)

int size_builtin_str = sizeof(builtin_str) / sizeof(builtin_str[0]); // number of built_in args

int main() 
{     
  printf("Welcome to psh!\n");
  return PSH_READ();
}

int PSH_EXIT(char **token_arr) 
{
  if (!token_arr[1])      
  {
    printf("bye bye PSH :D\n"); //handling empty args and freeing token array before leaving 
    free(token_arr);
    exit(0);
  }
  printf("bye bye PSH :D\n");
  int exit_code=atoi(token_arr[1]);
  free(token_arr);
  exit(exit_code);
}
int PSH_READ() {
  size_t n = 0;
  int run=1,isinbuilt=0;    
  char *inputline = NULL; // NULL is required to avoind conflicts with getline function
  while (run==1)          // if not done stack smashing occurs 
  { 
    printf("PSH $ ");
    if (getline(&inputline, &n, stdin) == -1) {
      perror("getline");
      free(inputline);
      return -1;
    }
    inputline[strcspn(inputline, "\n")] = '\0';
    
    // getline takes \n as a part of string when pressed enter this.
    // line is used to remove that \n and changing it blank space

    /* THIS IS NOW IMPLEMENTED AS A SEPERATE FUNCTION */

    // if (strcmp(inputline, "exit") == 0) { // checks if the input is exit to
    // quit
    //   // printf("bye bye PSH :D");
    //   free(inputline);
    //   exit(0);
    // }
    char **tokens = PSH_TOKENIZER(inputline);
    if (tokens != NULL) 
    {
      for (int i = 0; i < size_builtin_str; i++) 
      {
        if (!strcmp(tokens[0], builtin_str[i])) 
        {
          if(!strcmp(tokens[0],"exit"))
            free(inputline);  
          run = (*builtin_func[i])(tokens);
          isinbuilt=1;
        } // frees tokens
      }
      if (!isinbuilt) 
        run=PSH_EXEC_EXTERNAL(tokens);
      free(tokens);
    }
  } 
  free(inputline);
  return run;
}

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
  while (token != NULL)                 // making an array of tokens by seperating the line
  {                                     // one by one as delim as " "
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

  // while(token_arr[i]!=NULL)            //debugging tokens array
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
    if (pid == 0) 
    {
      // Child process
      if (execvp(token_arr[0], token_arr) == -1) //executes the binary file with args
      {
        perror("psh failed");
      }
      exit(EXIT_FAILURE);
    } else if (pid < 0) 
    {
      // Error forking
      perror("psh error");
    } else 
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
      }while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
  }
