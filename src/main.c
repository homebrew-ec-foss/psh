#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void PSH_READ();
char **PSH_TOKENIZER(char *line);
int PSH_EXEC_EXTERNAL(char **);

int main(int argc, char **argv) {
  printf("Welcome to psh!\n");
  // printf("%i\n", argc);
  PSH_READ();
  return 0;
}

void PSH_READ() {
  size_t n = 0;
  char *inputline =
      NULL;    // NULL is required to avoind conflicts with getline function
  while (69) { // if not done stack smashing occurs
    printf("PSH $ ");
    if (getline(&inputline, &n, stdin) == -1) {
      perror("getline");
      free(inputline);
      exit(1);
    }
    inputline[strcspn(inputline, "\n")] ='\0'; 
        // getline takes \n as a part of string when pressed enter this.
        // line is used to remove that \n and changing it blank space
    if (strcmp(inputline, "exit") == 0) 
    {                                           //checks if the input is exit to quit
      printf("bye bye PSH :D");
      free(inputline);
      exit(0);
    }

    char **tokens = PSH_TOKENIZER(inputline);
    if (tokens != NULL) {                       //frees tokens
      PSH_EXEC_EXTERNAL(tokens);
      free(tokens);
    }
  }
  free(inputline);
}

char **PSH_TOKENIZER(char *line) {
  int bufsize = 64, position = 0, i = 0;
  char **token_arr = malloc(bufsize * sizeof(char *));
  char *token;

  if (!token_arr) {
    fprintf(stderr, "psh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, " ");
  while (token != NULL) {               //making an array of tokens by seperating the line one by one as delim as " "
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
  token_arr[position] = NULL;           //terminating 

  
  // while(token_arr[i]!=NULL)            //testing tokens array
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
    if (pid == 0) {
        // Child process
        if (execvp(token_arr[0], token_arr) == -1) {
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
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
