// main.c
#include "psh.h"

// Unused Parameters
int main(int argc, char **argv, char **envp)
{
    // setenv("PS1_COLOR", "\033[0m", 1); // Set default color to reset

    // printf("\e[1;1H\e[2J"); // basically clears the screen
    getcwd(cwd, sizeof(cwd)); // home/$USER/psh
    strcpy(PATH, cwd);
    //add a PSH_SCRIPT call to load pshrc
    // PSH_SCRIPT("/home/pro696969/psh/.files/pshrc");

    if (argc == 2)
    {   
        return PSH_SCRIPT(argv[1]);
    }
    else
    {
        printf("Welcome to psh!\n");
        initialize_shell(cwd);
        return PSH_LOOP();
    }
    free_history();
    return 0;
}

int PSH_LOOP(void)
{
    size_t n = 0;
    int run = 1;
    char *inputline = NULL;
    strcpy(path_memory, cwd);
    strcat(path_memory, "/.files/MEMORY_HISTORY_FILE");

    while (run == 1)
    {
        handle_input(&inputline, &n, PATH);
        if (inputline[0] == '\0')
        {
            continue;
        }
        char path_session[PATH_MAX];
        get_session_path(path_session, sizeof(path_session), cwd);
        save_history(inputline,path_session);
        process_commands(inputline, &run);
    }
    free(inputline);
    return run;
}


// int PSH_SCRIPT(const char *file)
// {

//     FILE *script = fopen(file, "r");

//     int run = 1;
//     size_t n = 0;
//     char *inputline = NULL;
//     int result = 0;

//     if (script == NULL)
//     {
//         fprintf(stderr, "FILE open failed\n");
//         run = 0;
//         return -1;
//     }

//     while (run == 1)
//     {
//         if (getline(&inputline, &n, script) == -1)
//         {
//             if (feof(script))
//             {
//                 // Reached end of file
//                 // feof() doesn't actually detect the end of the file itself.
//                 // Instead, it reports whether a previous read operation has attempted
//                 // to read past the end of the file.
//                 break;
//             }
//             if (errno != 0)
//             {
//                 perror("getline");
//                 result = -1;
//             }
//             break;
//         }
        
//         // printf("getline is %s \n", inputline);

//         // handle_input(&inputline, &n, PATH);
//         // char *comment_pos = strchr(inputline, '#');
//         // if (comment_pos) {
//         //     *comment_pos = '\0';
//         // }
//         if (inputline[0] == '\0')
//         {
//             continue;
//         }
//         // char path_session[PATH_MAX];
//         // get_session_path(path_session, sizeof(path_session), cwd);
//         // save_history(inputline,path_session);
//         process_commands_script(inputline, &run);
//     }
//     free(inputline);
//     fclose(script);
//     return run;
// }


int PSH_SCRIPT(const char *file)
{

    FILE *script = fopen(file, "r");

    int run = 1;
    size_t n = 0;
    char *inputline = NULL;
    int result = 0;

    if (script == NULL)
    {
        fprintf(stderr, "FILE open failed\n");
        run = 0;
        return -1;
    }

    while (run == 1)
    {
        if (getline(&inputline, &n, script) == -1)
        {
            if (feof(script))
            {
                // Reached end of file
                // feof() doesn't actually detect the end of the file itself.
                // Instead, it reports whether a previous read operation has attempted
                // to read past the end of the file.
                break;
            }
            if (errno != 0)
            {
                perror("getline");
                result = -1;
            }
            break;
        }

        inputline[strcspn(inputline, "\n")] = '\0';

        // Ignore comments starting with '#'
        char *comment_pos = strchr(inputline, '#');
        if (comment_pos)
        {
            *comment_pos = '\0';
        }

        // Skip processing if the line becomes empty after stripping comments
        if (inputline[0] == '\0')
        {
            continue;
        }

        char **commands = split_commands(inputline);
        for (int i = 0; commands[i] != NULL; i++)
        {
            char **token_arr = PSH_TOKENIZER(commands[i]);
            if (token_arr[0] != NULL)
            {
                if (strchr(token_arr[0], '='))
                {
                    handle_env_variable(token_arr);
                    free_double_pointer(token_arr);
                    continue;
                }
                int isinbuilt = 0;
                for (int j = 0; j < size_builtin_str; j++)
                {
                    if (strcmp(token_arr[0], builtin_str[j]) == 0)
                    {
                        if (!strcmp(token_arr[0], "exit"))
                        {
                            run = (*builtin_func[j])(token_arr);
                            free_double_pointer(token_arr);
                            free_double_pointer(commands);
                            free(inputline);
                            fclose(script);
                            return run;
                        }
                        run = (*builtin_func[j])(token_arr);
                        isinbuilt = 1;
                        break;
                    }
                }
                if (!isinbuilt)
                {
                    run = PSH_EXEC_EXTERNAL(token_arr);
                }
                free_double_pointer(token_arr);
            }
        }
        free_double_pointer(commands);
    }

    free(inputline);
    fclose(script);
    return result;
}
