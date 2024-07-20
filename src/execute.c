#include "psh.h"
#include <stdio.h>

// Helper function to split the input line by ';'

/*char **split_commands(char *input)
{
    size_t bufsize = 64, position = 0;
    char **commands = malloc(bufsize * sizeof(char *));
    char *command;

    if (!commands)
    {
        fprintf(stderr, "psh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    command = strtok(input, ";");
    while (command != NULL)
    {
        commands[position] = malloc((strlen(command) + 1) * sizeof(char));
        if (!commands[position])
        {
            fprintf(stderr, "psh: allocation error\n");
            exit(EXIT_FAILURE);
        }
        strcpy(commands[position], command);
        position++;

        if (position >= bufsize)
        {
            bufsize += 64;
            commands = realloc(commands, bufsize * sizeof(char *));
            if (!commands)
            {
                fprintf(stderr, "psh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        command = strtok(NULL, ";");
    }
    commands[position] = NULL;
    return commands;
}*/

char **split_commands(char *input)
{
    size_t bufsize = 64, position = 0;
    char **commands = malloc(bufsize * sizeof(char *));
    char *command_start = input;
    int in_for_loop = 0;

    if (!commands)
    {
        fprintf(stderr, "psh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (char *c = input; *c != '\0'; c++)
    {
        if (!in_for_loop && *c == ';')
        {
            commands[position] = malloc((c - command_start + 1) * sizeof(char));
            if (!commands[position])
            {
                fprintf(stderr, "psh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            strncpy(commands[position], command_start, c - command_start);
            commands[position][c - command_start] = '\0';
            position++;

            if (position >= bufsize)
            {
                bufsize += 64;
                commands = realloc(commands, bufsize * sizeof(char *));
                if (!commands)
                {
                    fprintf(stderr, "psh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }

            command_start = c + 1;
        }
        else if (strncmp(c, "for ", 4) == 0)
        {
            in_for_loop++;
        }
        else if (in_for_loop && strncmp(c, "done", 4) == 0)
        {
            in_for_loop--;
            c += 3; // Move past "done"
        }
    }

    if (command_start != input + strlen(input))
    {
        commands[position] = strdup(command_start);
        if (!commands[position])
        {
            fprintf(stderr, "psh: allocation error\n");
            exit(EXIT_FAILURE);
        }
        position++;
    }

    commands[position] = NULL;
    return commands;
}

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
    while (token != NULL)
    {
        size_t len = strlen(token);
        for (i = 0; i < strlen(token); i++)
        {
            if ((has_quote == 0) && (token[i] == '"' || token[i] == '\''))
            {
                quote = token[i];
                has_quote = 1;
                break;
            }
            else if (token[i] == quote)
            {
                has_quote = 1;
                break;
            }
            else
            {
                has_quote = 0;
            }
        }

        if (has_quote == 1)
        {
            for (size_t j = i + 1; j < strlen(token); j++)
            {
                token[j - 1] = token[j];
            }
            token[strlen(token) - 1] = '\0';
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
        }

        if (position >= bufsize)
        {
            bufsize += 64;
            token_arr = realloc(token_arr, bufsize * sizeof(char *));
            if (!token_arr)
            {
                fprintf(stderr, "psh: allocation error\n");
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
    return token_arr;
}

int PSH_EXEC_EXTERNAL(char **token_arr)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        if (execvp(token_arr[0], token_arr) == -1)
        {
            fprintf(stdout, "psh: No command found: %s\n", token_arr[0]);
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("psh error");
    }
    else
    {
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

void handle_input(char **inputline, size_t *n)
{
    char last_component[PATH_MAX];
    get_last_path_component(PATH, last_component);

    if (strcmp(PATH, "/") == 0)
    {
        printf("%s@PSH → %s $ ", getenv("USER"), "/");
    }
    else
    {
        printf("%s@PSH → %s $ ", getenv("USER"), last_component);
    }

    if (getline(inputline, n, stdin) == -1)
    {
        perror("getline");
        free(*inputline);
        exit(EXIT_FAILURE);
    }

    (*inputline)[strcspn(*inputline, "\n")] = '\0';

    char *comment_pos = strchr(*inputline, '#');
    if (comment_pos)
    {
        *comment_pos = '\0';
    }
}

void save_history(const char *inputline)
{
    FILE *fp1, *fp2;

    char path_memory[PATH_MAX];
    strcpy(path_memory, cwd);
    strcat(path_memory, "/.files/MEMORY_HISTORY_FILE");
    fp1 = fopen(path_memory, "a");

    char path_session[PATH_MAX];
    strcpy(path_session, cwd);
    strcat(path_session, "/.files/SESSION_HISTORY_FILE");
    fp2 = fopen(path_session, "a");

    if (fp1 == NULL || fp2 == NULL)
    {
        perror("Error:");
        if (fp1)
            fclose(fp1);
        if (fp2)
            fclose(fp2);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(fp1, "%s\n", inputline);
        fprintf(fp2, "%s\n", inputline);
        fclose(fp1);
        fclose(fp2);
    }
}

void process_commands(char *inputline, int *run){
    char **commands = split_commands(inputline);

    for (int i = 0; commands[i] != NULL; i++)
    {
        char **token_arr = PSH_TOKENIZER(commands[i]);

        if (token_arr[0] != NULL)
        {
            execute_command(token_arr, run);
        }

        free_double_pointer(token_arr);
    }

    free_double_pointer(commands);
}

void execute_command(char **token_arr, int *run)
{
    if (strchr(token_arr[0], '='))
    {
        char *var_name = strtok(token_arr[0], "=");
        char *var_value = strtok(NULL, "=");

        if (var_value != NULL)
        {
            // if (num_vars < MAX_VARS) {
            //     strcpy(global_vars[num_vars].var_name, var_name);
            //     strcpy(global_vars[num_vars].var_value, var_value);
            //     num_vars++;
            // } else {
            //     fprintf(stderr, "PSH: Invalid variable assignment\n");
            // }
            setenv(var_name, var_value, 1);
            return;
        }
    }

    for (int j = 0; j < size_builtin_str; j++)
    {
        if (strcmp(token_arr[0], builtin_str[j]) == 0)
        {

            *run = (*builtin_func[j])(token_arr);
            return;
        }
    }

    if (!contains_wildcard(token_arr))
    {
        *run = PSH_EXEC_EXTERNAL(token_arr);
    }

    else
    {
        if (strchr(token_arr[0], '?') || strchr(token_arr[0], '?'))
        {
            fprintf(stdout, "psh: No command found: %s\n", token_arr[0]);
        }
        else
        {
            // func to handle wildcards
            handle_wildcard(token_arr[1]);
        }
    }
}
