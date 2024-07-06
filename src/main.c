#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 1024
#define SESSION_HISTORY_FILE "session_history_file.txt"
#define GLOBAL_HISTORY_FILE "history_file.txt"

int PSH_READ();
char **PSH_TOKENIZER(char *);
int PSH_EXEC_EXTERNAL(char **);
int PSH_EXIT(char **);
int PSH_CD(char **);
int PSH_ECHO(char **);
int PSH_PWD(char **);
int PSH_HISTORY(char **);

void delete_file(const char *);
// if u want to continue execution return 1 in built_in funcs

static char PREV_DIR[1024];

char *builtin_str[] = {"exit", "cd", "echo", "pwd", "history"};

int (*builtin_func[])(char **) = {
    &PSH_EXIT, &PSH_CD, &PSH_ECHO, &PSH_PWD,
    &PSH_HISTORY}; // function pointer that takes in array of strings and return
                   // an int)

int size_builtin_str =
    sizeof(builtin_str) / sizeof(builtin_str[0]); // number of built_in args

int main(int argc, char **argv, char **envp) {
  printf("Welcome to psh!\n");

  // printf("%s@", getenv("USER"));
  // for (int i=0; envp[i]!=NULL; i++) {  // accessing all env variables
  //       printf("%d: %s\n", i, envp[i]);
  //   }

  return PSH_READ();
}

int PSH_EXIT(char **token_arr) {
  if (!token_arr[1]) {
    printf("bye bye PSH :D\n"); // handling empty args and freeing token array
                                // before leaving
    free(token_arr);
    delete_file(SESSION_HISTORY_FILE);
    return 0;
    // exit(0);
  }
  printf("bye bye PSH :D\n");
  int exit_code = atoi(token_arr[1]);
  free(token_arr);
  delete_file(SESSION_HISTORY_FILE);
  return exit_code;
  // exit(exit_code);
}

int PSH_CD(char **token_arr) {
  char current_dir[1024];

  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    perror("PSH: getcwd() error");
    return 1;
  }

  char *home = malloc(1024 * sizeof(char *));
  if (token_arr[1] == NULL || (strcmp(token_arr[1], "~") == 0)) {
    home = getenv("HOME");
    if (chdir(home) == -1) {
      perror("PSH: cd");
      free(home);
      return 1;
    }
  }

  else if (strcmp(token_arr[1], "-") == 0) {
    if (PREV_DIR[0] == '\0') {
      fprintf(stderr, "PSH: No previous directory\n");
      return 1;
    }
    // printf("%s\n", PREV_DIR); debugging
    if (chdir(PREV_DIR) == -1) {
      perror("PSH: chdir() error");
      return 1;
    }
  }

  else {
    if (chdir(token_arr[1]) == -1) {
      fprintf(stderr, "PSH: no such file/directory exists\n");
      return 1;
    }
  }
  // Update PREV_DIR after successful directory change
  strcpy(PREV_DIR, current_dir);

  // free(home);
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

  // printf("%s\n", token_arr[1]); //debugging
  // printf("Printing current working directory\n"); //debugging

  char cwd[1024];
  char rpath[1024];
  // printf("%d\n", PATH_MAX);

  if (token_arr[1] == NULL ||
      strcmp(token_arr[1], "-L") == 0) // Default pwd and pwd -L
  {
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
      printf("%s\n", cwd);
    } else {
      perror("PSH: getcwd() error");
    }
  } else if ((strcmp(token_arr[1], "-P")) == 0) {
    if (realpath(cwd, rpath) != NULL) // pwd -P
    {
      printf("%s\n", rpath);
    } else {
      perror("PSH: realpath() error");
    }
  } else {
    fprintf(stderr, "Unknown option: %s\n", token_arr[1]);
  }
  return 1;
}

// Function to read lines from a specific file
void read_lines(const char *filename) {
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t len = 0;
  size_t line_no = 1;

  if (file == NULL) {
    perror("Error:");
    return;
  }

  while (getline(&line, &len, file) != -1) {
    printf("%zu %s", line_no, line);
    line_no++;
  }

  fclose(file);

  if (line) {
    free(line);
  }
}

// Function to append new lines from a specific file
// void append_lines(const char *source_file, const char *dest_file) {
//   FILE *source = fopen(source_file, "r");
//   FILE *dest = fopen(dest_file, "a");
//   char *line = NULL;
//   size_t len = 0;

//   if (source == NULL || dest == NULL) {
//     perror("Error:");
//     return;
//   }

//   while (getline(&line, &len, source) != -1) {
//     fputs(line, dest);
//   }

//   fclose(source);
//   fclose(dest);

//   if (line) {
//     free(line);
//   }
// }

// Function to remove a specific line from the file
void remove_line(const char *filename, size_t line_to_remove) {
  FILE *file = fopen(filename, "r");
  FILE *tempfile = fopen("temp.txt", "w");
  char *line = NULL;
  size_t len = 0;
  size_t line_no = 1;

  if (file == NULL || tempfile == NULL) {
    perror("Error:");
    return;
  }

  while (getline(&line, &len, file) != -1) {
    if (line_no != line_to_remove) {
      fputs(line, tempfile);
    }
    line_no++;
  }

  fclose(file);
  fclose(tempfile);

  remove(filename);
  rename("temp.txt", filename);

  if (line) {
    free(line);
  }
}

void delete_file(const char *filename) {
  if (remove(filename) != 0) {
    perror("Error deleting file");
  }
}

void clear_session_history() {
  FILE *global_fp, *session_fp, *temp_fp;
  char *global_line = NULL, *session_line = NULL;
  size_t global_len = 0, session_len = 0;
  ssize_t global_read, session_read;
  int found;

  // Open the files
  global_fp = fopen(GLOBAL_HISTORY_FILE, "r");
  session_fp = fopen(SESSION_HISTORY_FILE, "r");
  temp_fp = fopen("temp_history_file.txt", "w");

  if (global_fp == NULL || session_fp == NULL || temp_fp == NULL) {
    perror("Error opening files");
    exit(EXIT_FAILURE);
  }

  // Read global history and remove session history lines
  while ((global_read = getline(&global_line, &global_len, global_fp)) != -1) {
    rewind(session_fp); // Reset session file pointer for each global line
    found = 0;
    while ((session_read = getline(&session_line, &session_len, session_fp)) !=
           -1) {
      if (strcmp(global_line, session_line) == 0) {
        found = 1;
        break;
      }
    }
    if (!found) {
      fprintf(temp_fp, "%s", global_line);
    }
  }

  // Free allocated memory and close files
  free(global_line);
  free(session_line);
  fclose(global_fp);
  fclose(session_fp);
  fclose(temp_fp);

  // Replace global history file with the updated one
  remove(GLOBAL_HISTORY_FILE);
  rename("temp_history_file.txt", GLOBAL_HISTORY_FILE);

  // Clear session history file
  temp_fp = fopen(SESSION_HISTORY_FILE, "w");
  fclose(temp_fp);
  printf("Session history cleared.\n");
}

char *expand_history(const char *arg, FILE *history_file) {
  char line[MAX_LINE_LENGTH];
  char *expanded = NULL;

  if (arg[0] == '!') {
    int event_number = atoi(arg + 1);
    int current_line = 0;

    while (fgets(line, sizeof(line), history_file) != NULL) {
      current_line++;
      if (current_line == event_number) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;
        expanded = strdup(line);
        break;
      }
    }
  }

  return expanded ? expanded : strdup(arg);
}

int PSH_HISTORY(char **token_arr) {
  FILE *fp;
  size_t size = 0;
  size_t line_no = 1;
  size_t ch_read;
  size_t n;

  if (fp == NULL) { 
    perror("Error:");
    return -1;
  } else {
    if (token_arr[1] == NULL) {
      n = 0;
    } else if ((strcmp(token_arr[1], "-c")) == 0) {
      n = 1;
    } else if ((strcmp(token_arr[1], "-d")) == 0) {
      n = 2;
    } else if ((strcmp(token_arr[1], "-a")) == 0) {
      n = 3;
    } else if ((strcmp(token_arr[1], "-n")) == 0) {
      n = 4;
    } else if ((strcmp(token_arr[1], "-r")) == 0) {
      n = 5;
    } else if ((strcmp(token_arr[1], "-w")) == 0) {
      n = 6;
    } else if ((strcmp(token_arr[1], "-p")) == 0) {
      n = 7;
    } else if ((strcmp(token_arr[1], "-s")) == 0) {
      n = 8;
    } else {
      printf("psh: history: numeric argument required");
      return 1;
    }

    switch (n) {
    case 0: {
      read_lines(GLOBAL_HISTORY_FILE);
      break;
    }

    case 1:
      clear_session_history();
      break;

    case 2: // -d option
    {
      FILE *fp1 = fopen(GLOBAL_HISTORY_FILE, "r");
      FILE *fp2 = fopen(SESSION_HISTORY_FILE, "r");

      if (fp1 == NULL || fp2 == NULL) {
        perror("Error opening history files");
        if (fp1)
          fclose(fp1);
        if (fp2)
          fclose(fp2);
        return -1;
      }

      if (token_arr[2] == NULL) {
        printf("psh: history -d requires a line number\n");
        fclose(fp1);
        fclose(fp2);
        return 1;
      }

      // Count total lines in global history
      size_t total_lines = 0;
      char buffer[1024];
      while (fgets(buffer, sizeof(buffer), fp1) != NULL) {
        total_lines++;
      }
      rewind(fp1);

      // Parse the offset
      long offset = strtol(token_arr[2], NULL, 10);
      size_t line_to_remove;

      if (offset > 0) {
        line_to_remove = (size_t)offset;
      } else if (offset < 0) {
        // Handle negative offset
        if ((size_t)(-offset) > total_lines) {
          printf("psh: history -d: %ld: history position out of range\n",
                 offset);
          fclose(fp1);
          fclose(fp2);
          return 1;
        }
        line_to_remove = total_lines + offset + 1;
      } else {
        printf("psh: history -d: 0: history position out of range\n");
        fclose(fp1);
        fclose(fp2);
        return 1;
      }

      // Check if line_to_remove is within range
      if (line_to_remove > total_lines) {
        printf("psh: history -d: %zu: history position out of range\n",
               line_to_remove);
        fclose(fp1);
        fclose(fp2);
        return 1;
      }

      // Remove the line from both files
      remove_line(GLOBAL_HISTORY_FILE, line_to_remove);
      remove_line(SESSION_HISTORY_FILE, line_to_remove);

      printf("Removed line %zu from history.\n", line_to_remove);

      fclose(fp1);
      fclose(fp2);
      break;
    }
    case 3:
      // The lines are usually always appended to the global history file
      break;

    case 4:
    //To be implemented later

      break;

    case 5: //-r
    {
      FILE *fp1 = fopen(GLOBAL_HISTORY_FILE, "r");
      FILE *fp2 = fopen(SESSION_HISTORY_FILE, "a");

      if (fp1 == NULL || fp2 == NULL) {
        perror("Error opening history files");
        if (fp1)
          fclose(fp1);
        if (fp2)
          fclose(fp2);
        return -1;
      }

      char *line = NULL;
      size_t len = 0;
      ssize_t read;

      // Read each line from the global history file and append it to the
      // session history file
      while ((read = getline(&line, &len, fp1)) != -1) {
        fputs(line, fp2);
      }

      if (line) {
        free(line);
      }

      printf("History file read and appended to the current history list.\n");

      // Now, display the updated history
      rewind(fp2);
      read_lines(GLOBAL_HISTORY_FILE);

      fclose(fp1);
      fclose(fp2);
      break;
    }

    case 6: { //TO BE MODIFIED
      FILE *global_fp, *session_fp, *temp_fp;
      char *global_line = NULL, *session_line = NULL;
      size_t global_len = 0, session_len = 0;
      ssize_t global_read, session_read;
      int found;

      // Open the files
      global_fp = fopen(GLOBAL_HISTORY_FILE, "a+");
      session_fp = fopen(SESSION_HISTORY_FILE, "r");

      if (global_fp == NULL || session_fp == NULL) {
        perror("Error opening files");
        exit(EXIT_FAILURE);
      }

      // Write session history to global history if not already present
      while ((session_read =
                  getline(&session_line, &session_len, session_fp)) != -1) {
        rewind(global_fp); // Reset global file pointer for each session line
        found = 0;
        while ((global_read = getline(&global_line, &global_len, global_fp)) !=
               -1) {
          if (strcmp(global_line, session_line) == 0) {
            found = 1;
            break;
          }
        }
        if (!found) {
          fprintf(global_fp, "%s", session_line);
        }
      }
      break;
    }
    case 7: // history -p !line_no
    {
      FILE *global_history = fopen(GLOBAL_HISTORY_FILE, "r");
      FILE *session_history = fopen(SESSION_HISTORY_FILE, "r");

      if (!global_history || !session_history) {
        perror("Error opening history files");
        return 1;
      }

      // Skip the "history" and "-p" tokens
      int i = 2;
      while (token_arr[i] != NULL) {
        char *expanded = expand_history(token_arr[i], global_history);
        if (!expanded) {
          expanded = expand_history(token_arr[i], session_history);
        }

        printf("%s\n", expanded);
        free(expanded);
        i++;
      }

      fclose(global_history);
      fclose(session_history);
      return 1;

      break;
    }

    case 8: // read a string to history
    {
      FILE *fp1 = fopen(GLOBAL_HISTORY_FILE, "a+");
      FILE *fp2 = fopen(SESSION_HISTORY_FILE, "a+");

      int size = 0;
      while (token_arr[size] != NULL) {
        size++;
      }

      char filename[100];
      char ch;
      int line_count_global = 0;
      int line_count_session = 0;

      if (fp1 == NULL || fp2 == NULL) {
        printf("Could not open file\n");
        return 1;
      }

      // Counting the lines for the global history file
      while ((ch = fgetc(fp1)) != EOF) {
        if (ch == '\n') {
          line_count_global++;
        }
      }

      // Counting the lines for the session history file
      while ((ch = fgetc(fp2)) != EOF) {
        if (ch == '\n') {
          line_count_session++;
        }
      }

      for (int a = 2; a < size; a++) // history -s word1 word2
      {
        fprintf(fp1, "%s ", token_arr[a]);
        fprintf(fp2, "%s ", token_arr[a]);
      }

      fprintf(fp1, "\n");
      fprintf(fp2, "\n");

      // printf("%d %d\n", line_count_global,line_count_session);
      // remove_line(GLOBAL_HISTORY_FILE, line_count_global);
      // remove_line(SESSION_HISTORY_FILE, line_count_session);
      // printf("%d %d\n", line_count_global,line_count_session);
      fclose(fp1);
      fclose(fp2);
      break;
    }

    default:
      printf("psh: history: option not implemented\n");
      break;
    }
  }
  return 1;
}

int PSH_READ() {
  size_t n = 0;
  int run = 1;
  char *inputline =
      NULL;        // NULL is required to avoind conflicts with getline function
  while (run == 1) // if not done stack smashing occurs
  {
    printf("%s@PSH $ ", getenv("USER"));
    if (getline(&inputline, &n, stdin) == -1) {
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

    fp1 = fopen(GLOBAL_HISTORY_FILE, "a");
    fp2 = fopen(SESSION_HISTORY_FILE, "a");

    // const char *exception1 = "-s";
    // const char *exception2 = "-p";
    // char *excp_res_1 = strstr(inputline,exception1);
    // char *excp_res_2 = strstr(inputline,exception2);

    if (fp1 == NULL && fp2 == NULL) {
      perror("Error:");
      return -1;
    } else {
      // if(excp_res_1==NULL || excp_res_2==NULL){
      fprintf(fp1, "%s\n", inputline);
      fprintf(fp2, "%s\n", inputline);
      // }
    }
    fclose(fp1);
    fclose(fp2);

    char **token_arr = PSH_TOKENIZER(inputline);
    if (token_arr[0] != NULL) // fixed \n giving seg fault
    {
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
  while (token != NULL) // making an array of token_arr by separating the line
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
    if (execvp(token_arr[0], token_arr) ==
        -1) // executes the binary file with args
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
