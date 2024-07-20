// helpers.c
#include "psh.h"
#include <stdio.h>
#include <string.h>
// char path_memory[PATH_MAX];

void free_double_pointer(char **array)
{
    if (array == NULL)
    {
        return;
    }
    for (int i = 0; array[i] != NULL; i++)
    {
        free(array[i]);
    }
    free(array);
}

void remove_last_component(char *path)
{
    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL)
    {
        *last_slash = '\0';
    }
}

void get_last_path_component(const char *full_path, char *last_component)
{
    char *last_slash = strrchr(full_path, '/');
    if (last_slash != NULL)
    {
        strcpy(last_component, last_slash + 1); // +1 to skip the '/'
    }
    else
    {
        strcpy(last_component, full_path); // Fallback in case there's no '/'
    }
}

// Function to read lines
void read_lines(const char *filename, int low_lim, int up_lim)
{
    FILE *file = fopen(filename, "r");
    char *line = NULL;
    size_t len = 0;
    size_t line_no = 1;

    if (file == NULL)
    {
        perror("Error:");
        return;
    }

    while ((getline(&line, &len, file) != -1) && line != NULL)
    {
        if (line_no >= low_lim && line_no <= up_lim)
        {
            printf("%zu %s", line_no, line);
        }
        line_no++;
        if (line_no > up_lim)
        {
            break;
        }
    }

    fclose(file);

    if (line)
    {
        free(line);
    }
}

// Function to read lines without displaying the numbers
void read_lines_wo_no(const char *filename, int low_lim, int up_lim)
{
    FILE *file = fopen(filename, "r");
    char *line = NULL;
    size_t len = 0;
    size_t line_no = 1;

    if (file == NULL)
    {
        perror("Error:");
        return;
    }

    while (getline(&line, &len, file) != -1)
    {
        if (line_no >= low_lim && line_no <= up_lim)
        {
            printf("%s", line);
        }
        line_no++;
        if (line_no > up_lim)
        {
            break;
        }
    }

    fclose(file);

    if (line)
    {
        free(line);
    }
}

// Function to count lines
int count_lines(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    int total_lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        total_lines++;
    }
    fclose(fp);
    return total_lines;
}

void read_lines_reverse(const char *filename, int low_lim, int up_lim)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error:");
        return;
    }

    char **lines = NULL;
    size_t size = 0;
    size_t capacity = 10;
    lines = malloc(capacity * sizeof(char *));
    if (lines == NULL)
    {
        perror("Error:");
        fclose(file);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    size_t line_no = 1;

    while (getline(&line, &len, file) != -1)
    {
        if (line_no >= low_lim && line_no <= up_lim)
        {
            if (size == capacity)
            {
                capacity *= 2;
                lines = realloc(lines, capacity * sizeof(char *));
                if (lines == NULL)
                {
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
        if (line_no > up_lim)
        {
            break;
        }
    }

    fclose(file);
    if (line)
    {
        free(line);
    }

    for (size_t i = size; i > 0; i--)
    {
        printf("%s", lines[i - 1]);
        free(lines[i - 1]);
    }

    free(lines);
}

// Function to remove a specific line
void remove_line(const char *filename, size_t line_to_remove)
{
    FILE *file = fopen(filename, "r");
    FILE *tempfile = fopen("temp.txt", "w");
    char *line = NULL;
    size_t len = 0;
    size_t line_no = 1;

    if (file == NULL || tempfile == NULL)
    {
        perror("Error:");
        return;
    }

    while (getline(&line, &len, file) != -1)
    {
        if (line_no != line_to_remove)
        {
            fputs(line, tempfile);
        }
        line_no++;
    }

    fclose(file);
    fclose(tempfile);

    remove(filename);
    rename("temp.txt", filename);

    if (line)
    {
        free(line);
    }
}

void delete_file(const char *filename)
{
    if (remove(filename) != 0)
    {
        perror("Error deleting file");
    }
}

void clear_session_history()
{
    char MEMORY_HISTORY_FILE[PATH_MAX];
    char SESSION_HISTORY_FILE[PATH_MAX];

    strcpy(path_memory, cwd);
    strcat(path_memory, "/.files/MEMORY_HISTORY_FILE");
    strcpy(MEMORY_HISTORY_FILE, path_memory);

    char path_session[PATH_MAX];
    strcpy(path_session, cwd);
    strcat(path_session, "/.files/SESSION_HISTORY_FILE");
    strcpy(SESSION_HISTORY_FILE, path_session);

    FILE *global_fp, *session_fp, *temp_fp;
    char *global_line = NULL, *session_line = NULL;
    size_t global_len = 0, session_len = 0;
    ssize_t global_read, session_read;
    int found;

    // Open the files
    global_fp = fopen(MEMORY_HISTORY_FILE, "r");
    session_fp = fopen(SESSION_HISTORY_FILE, "r");
    temp_fp = fopen("temp_history_file.txt", "w");

    if (global_fp == NULL || session_fp == NULL || temp_fp == NULL)
    {
        perror("Error opening files");
        exit(EXIT_FAILURE);
    }

    // Read global history and remove session history lines
    while ((global_read = getline(&global_line, &global_len, global_fp)) != -1)
    {
        rewind(session_fp); // Reset session file pointer for each global line
        found = 0;
        while ((session_read = getline(&session_line, &session_len, session_fp)) !=
               -1)
        {
            if (strcmp(global_line, session_line) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
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

char *expand_history(const char *arg, FILE *history_file)
{
    char line[MAX_LINE_LENGTH];
    char *expanded = NULL;

    if (arg[0] == '!')
    {
        int event_number = atoi(arg + 1);
        int current_line = 0;

        while (fgets(line, sizeof(line), history_file) != NULL)
        {
            current_line++;
            if (current_line == event_number)
            {
                // Removing newline character if present
                // line[strcspn(line, "\n")] = 0;
                expanded = strdup(line);
                break;
            }
        }
    }

    return expanded ? expanded : strdup(arg);
}

void read_lines_reverse_wo_no(const char *filename, int low_lim, int up_lim)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error:");
        return;
    }

    char **lines = NULL;
    size_t size = 0;
    size_t capacity = 10;
    lines = malloc(capacity * sizeof(char *));
    if (lines == NULL)
    {
        perror("Error:");
        fclose(file);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    size_t line_no = 1;

    while (getline(&line, &len, file) != -1)
    {
        if (line_no >= low_lim && line_no <= up_lim)
        {
            if (size == capacity)
            {
                capacity *= 2;
                lines = realloc(lines, capacity * sizeof(char *));
                if (lines == NULL)
                {
                    perror("Error:");
                    free(line);
                    fclose(file);
                    return;
                }
            }
            // Storing the line without the line number
            lines[size++] = strdup(line);
        }
        line_no++;
        if (line_no > up_lim)
        {
            break;
        }
    }

    fclose(file);
    if (line)
    {
        free(line);
    }

    for (size_t i = size; i > 0; i--)
    {
        printf("%s", lines[i - 1]);
        free(lines[i - 1]);
    }
    free(lines);
}

int compare_strings(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void sort_strings(char **strings, int num_strings)
{
    qsort(strings, num_strings, sizeof(char *), compare_strings);
}

int size_token_arr(char **token_arr)
{
    int i = 0;
    while (token_arr[i] != NULL)
    {
        i++;
    }
    return i;
}

bool contains_wildcard(char **token_arr)
{

    int size = size_token_arr(token_arr);
    for (int i = 0; i < size; i++)
    {
        if (strchr(token_arr[i], '?') ||
            strchr(token_arr[i], '*'))
        {
            return true;
        }
    }
    return false;
}

int handle_wildcard(char *pattern)
{
    char **found;
    glob_t glob_struct;
    int run;

    run = glob(pattern, GLOB_ERR, NULL, &glob_struct);

    // check for errors
    if (run != 0)
    {
        if (run == GLOB_NOMATCH)
            fprintf(stderr, "No matches\n");

        else
            fprintf(stderr, "Some kinda glob error\n");
        // exit(1);
        return 1;
    }

    /* success, output found filenames */
    printf("\n");
    printf("Found %zu filename matches\n", glob_struct.gl_pathc);
    found = glob_struct.gl_pathv;

    while (*found)
    {
        printf("%s\n", *found);
        found++;
    }
    printf("\n");
    return 1;
}

char *find_closing_done(char *start)
{
    int depth = 0;
    char *pos = start;
    while (*pos)
    {
        if (strncmp(pos, "for ", 4) == 0)
        {
            depth++;
        }
        else if (strncmp(pos, "done", 4) == 0)
        {
            if (depth == 0)
            {
                return pos;
            }
            depth--;
        }
        pos++;
    }
    return NULL;
}

void process_nested_loops(char *command, int *run)
{
    char *current_command = command;
    while ((current_command = strstr(current_command, "for ")) != NULL)
    {
        current_command = process_for_loop(current_command, run);
        if (!current_command || *run == 0)
        {
            return;
        }
    }
}

char *process_for_loop(char *loop_command, int *run)
{
    char *saveptr;
    char *start = strstr(loop_command, "for ");
    if (!start)
    {
        fprintf(stderr, "Error: Missing 'for' keyword\n");
        return NULL;
    }

    start += 4;
    char *var_name = strtok(start, " ");
    if (!var_name)
    {
        fprintf(stderr, "Error: Missing variable name\n");
        return NULL;
    }

    char *in = strtok(NULL, " ");
    if (!in || strcmp(in, "in") != 0)
    {
        fprintf(stderr, "Error: Missing 'in' keyword\n");
        return NULL;
    }

    char *values = strtok(NULL, ";");
    if (!values)
    {
        fprintf(stderr, "Error: Missing values after 'in'\n");
        return NULL;
    }

    char *do_keyword = strtok(NULL, " ");
    if (!do_keyword || strcmp(do_keyword, "do") != 0)
    {
        fprintf(stderr, "Error: Missing 'do' keyword\n");
        return NULL;
    }

    char *commands_start = do_keyword + 3;
    char *commands_end = find_closing_done(commands_start);
    if (!commands_end)
    {
        fprintf(stderr, "Error: Missing 'done' keyword\n");
        return NULL;
    }

    size_t commands_length = commands_end - commands_start;
    char *commands = malloc(commands_length + 1);
    if (!commands)
    {
        fprintf(stderr, "Error: Allocation error for commands\n");
        return NULL;
    }

    strncpy(commands, commands_start, commands_length);
    commands[commands_length] = '\0';

    char *value = strtok_r(values, " ", &saveptr);
    while (value)
    {
        char command_block[PATH_MAX];
        snprintf(command_block, PATH_MAX, "%s=%s; %s", var_name, value, commands);
        value = strtok_r(NULL, " ", &saveptr);
        process_commands(command_block, run);
        if (*run == 0)
        {
            free(commands);
            return commands_end + 4; // Return the position after "done"
        }
    }
    free(commands);
    return commands_end + 4; // Return the position after "done"
}

void get_last_line(char **inputline) {

  last_command_up = 1;
//   printf("entering getlastline\n");
  FILE *fp1 = fopen(path_memory, "r");

  if (fp1 == NULL) {
    perror("Error opening file");
    return;
  }

  char line[MAX_LINE_LENGTH] = "";
  char lastLine[MAX_LINE_LENGTH] = "";
  char secondLastLine[MAX_LINE_LENGTH] = "";
  // char thirdLastLine[MAX_LINE_LENGTH] = "";

//   printf("path is %s\n", path_memory);

  // Read each line and store the last one in lastLine
  while (fgets(line, sizeof(line), fp1)) {
    // strcpy(thirdLastLine,secondLastLine);
    strcpy(secondLastLine, lastLine);
    strcpy(lastLine, line);
  }

  //   printf("testt\n");
  //   Close the file
  system(lastLine);  // fix this later on
  
//   strcpy(*inputline, lastLine);
//   printf("last linee : %s\n",*inputline);

//   last_command_up = 0;
  fclose(fp1);

  // Print the last line
  // printf("Last command: %s\n", lastLine);
  // fflush(stdin);
}