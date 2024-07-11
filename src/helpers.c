// helpers.c
#include "psh.h"

char *commonSuffix(char *str1, char *str2) {
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int minLength = len1 < len2 ? len1 : len2;
  int i;

  // Find the length of the common suffix
  for (i = 0; i < minLength; i++) {
    if (str1[len1 - 1 - i] != str2[len2 - 1 - i]) {
      break;
    }
  }

  // Allocate memory for the result string
  char *result = (char *)malloc((i + 1) * sizeof(char));
  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  // Copy the common suffix into the result string
  strncpy(result, str1 + len1 - i, i);
  result[i] = '\0';

  return result;
}

void remove_last_component(char *path) {
  char *last_slash = strrchr(path, '/');
  if (last_slash != NULL) {
    *last_slash = '\0';
  }
}

int resolve_and_manage_symlink(char *localdir, char *resolved_path) {
  if (realpath(localdir, resolved_path) == NULL) {
    if (errno == ENOENT) {
      fprintf(stderr, "PSH: no such file or directory: %s\n", localdir);
    } else {
      perror("PSH: realpath() error");
    }
    return -1;
  }

  if (chdir(resolved_path) == -1) {
    perror("PSH: chdir() error");
    return -1;
  }

  return 0;
}

char *helper_cd_func1(const char *str1, const char *str2) {
  static char result[256]; // Use static to return a pointer
  char temp1[256], temp2[256];
  char *tokens1[64], *tokens2[64];
  int len1 = 0, len2 = 0;

  // Copy the input strings to temporary variables
  strcpy(temp1, str1);
  strcpy(temp2, str2);

  // Tokenize the first string by "/"
  char *token = strtok(temp1, "/");
  while (token != NULL) {
    tokens1[len1++] = token;
    token = strtok(NULL, "/");
  }

  // Tokenize the second string by "/"
  token = strtok(temp2, "/");
  while (token != NULL) {
    tokens2[len2++] = token;
    token = strtok(NULL, "/");
  }

  // Compare tokens from the end and find the point of divergence
  int i = len1 - 1;
  int j = len2 - 1;
  while (i >= 0 && j >= 0 && strcmp(tokens1[i], tokens2[j]) == 0) {
    i--;
    j--;
  }

  // Build the result string from the remaining tokens in str1
  result[0] = '\0'; // Initialize result as an empty string
  for (int k = 0; k <= i; k++) {
    strcat(result, "/");
    strcat(result, tokens1[k]);
  }

  return result;
}

// Function to read lines
void read_lines(const char *filename, int low_lim, int up_lim) {
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t len = 0;
  size_t line_no = 1;

  if (file == NULL) {
    perror("Error:");
    return;
  }

  while ((getline(&line, &len, file) != -1) && line != NULL) {
    if (line_no >= low_lim && line_no <= up_lim) {
      printf("%zu %s", line_no, line);
    }
    line_no++;
    if (line_no > up_lim) {
      break;
    }
  }

  fclose(file);

  if (line) {
    free(line);
  }
}

// Function to read lines without displaying the numbers
void read_lines_wo_no(const char *filename, int low_lim, int up_lim) {
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t len = 0;
  size_t line_no = 1;

  if (file == NULL) {
    perror("Error:");
    return;
  }

  while (getline(&line, &len, file) != -1) {
    if (line_no >= low_lim && line_no <= up_lim) {
      printf("%s", line);
    }
    line_no++;
    if (line_no > up_lim) {
      break;
    }
  }

  fclose(file);

  if (line) {
    free(line);
  }
}

// Function to count lines
int count_lines(const char *filename) {
  FILE *fp = fopen(filename, "r");
  int total_lines = 0;
  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    total_lines++;
  }
  fclose(fp);
  return total_lines;
}

void read_lines_reverse(const char *filename, int low_lim, int up_lim) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error:");
    return;
  }

  char **lines = NULL;
  size_t size = 0;
  size_t capacity = 10;
  lines = malloc(capacity * sizeof(char *));
  if (lines == NULL) {
    perror("Error:");
    fclose(file);
    return;
  }

  char *line = NULL;
  size_t len = 0;
  size_t line_no = 1;

  while (getline(&line, &len, file) != -1) {
    if (line_no >= low_lim && line_no <= up_lim) {
      if (size == capacity) {
        capacity *= 2;
        lines = realloc(lines, capacity * sizeof(char *));
        if (lines == NULL) {
          perror("Error:");
          free(line);
          fclose(file);
          return;
        }
      }
      // Storing the line with the line number
      asprintf(&lines[size++], "%zu %s", line_no, line);
    }
    line_no++;
    if (line_no > up_lim) {
      break;
    }
  }

  fclose(file);
  if (line) {
    free(line);
  }

  for (size_t i = size; i > 0; i--) {
    printf("%s", lines[i - 1]);
    free(lines[i - 1]);
  }

  free(lines);
}

// Function to remove a specific line
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
  global_fp = fopen(MEMORY_HISTORY_FILE, "r");
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
  remove(MEMORY_HISTORY_FILE);
  rename("temp_history_file.txt", MEMORY_HISTORY_FILE);

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
        // Removing newline character if present
        // line[strcspn(line, "\n")] = 0;
        expanded = strdup(line);
        break;
      }
    }
  }

  return expanded ? expanded : strdup(arg);
int compare_strings(const void *a, const void *b)
{
  return strcmp(*(const char **)a, *(const char **)b);
}

void sort_strings(char **strings, int num_strings)
{
  qsort(strings, num_strings, sizeof(char *), compare_strings);
}
