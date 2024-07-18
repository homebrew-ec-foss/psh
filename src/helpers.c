// helpers.c
#include "psh.h"

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

    char path_memory[PATH_MAX];
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
