#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int PSH_READ();
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);
int PSH_EXIT(char **);
int PSH_CD(char **);
int PSH_ECHO(char **);
int PSH_PWD(char **);
int PSH_HISTORY(char **);

// if u want to continue execution return 1 in built_in funcs

char *builtin_str[] = {"exit", "cd", "echo", "pwd", "history"};

int (*builtin_func[])(char **) = {
    &PSH_EXIT, &PSH_CD, &PSH_ECHO, &PSH_PWD, &PSH_HISTORY}; // function pointer that takes in array of strings and
                         // return an int)

int size_builtin_str = sizeof(builtin_str) / sizeof(builtin_str[0]); // number of built_in args

int main() {
  printf("Welcome to psh!\n");
  return PSH_READ();
}

int PSH_EXIT(char **token_arr) {
  if (!token_arr[1]) {
    printf("bye bye PSH :D\n"); // handling empty args and freeing token array
                                // before leaving
    free(token_arr);
    exit(0);
  }
  printf("bye bye PSH :D\n");
  int exit_code = atoi(token_arr[1]);
  free(token_arr);
  exit(exit_code);
}

int PSH_CD(char **token_arr) { //kk

  if (token_arr[1] == NULL) {
    fprintf(stderr, "PSH: expected argument to \"cd\"\n");
    return 1;
  }

  if (chdir(token_arr[1]) == -1) {
    fprintf(stderr, "PSH: no such file/directory exists\n");
  }

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
  //your code goes here SUMITHRA
  return 1;
}

int PSH_HISTORY(char **token_arr) {
  //your code goes here Alayna
  return 1;
} 

int PSH_READ() {
  size_t n = 0;
  int run = 1;

  char *inputline = NULL;        // NULL is required to avoind conflicts with getline function
  while (run == 1) // if not done stack smashing occurs
  {
  loopstart:
    printf("PSH $ ");
    if (getline(&inputline, &n, stdin) == -1) {
      perror("getline");
      free(inputline);
      return -1;
    }

    inputline[strcspn(inputline, "\n")] = '\0';
    if (!strcmp(inputline, "\0")) {
      goto loopstart;
    }
    // getline takes \n as a part of string when pressed enter this.
    // line is used to remove that \n and changing it blank space

    /* THIS IS NOW IMPLEMENTED AS A SEPERATE FUNCTION */

    // if (strcmp(inputline, "exit") == 0) { // checks if the input is exit to
    // quit
    //   // printf("bye bye PSH :D");
    //   free(inputline);
    //   exit(0);
    // }
    char **token_arr = PSH_TOKENIZER(inputline);

    if (token_arr != NULL) {
      int isinbuilt = 0;
      for (int i = 0; i < size_builtin_str; i++) {
        if (!strcmp(token_arr[0], builtin_str[i])) {
          // if(!strcmp(token_arr[0],"exit"))
          // {
          //   //  free(inputline);
          // }
          run = (*builtin_func[i])(token_arr);
          isinbuilt = 1;
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
  while (token != NULL) // making an array of token_arr by seperating the line
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
    if (execvp(token_arr[0], token_arr) == -1) // executes the binary file with args
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