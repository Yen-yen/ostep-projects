#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_PATHS 10
#define MAX_TOKENS 100
#define MAX_CMD_LEN 256
int main(int argc, char *argv[])
{
    FILE *fptr = NULL;
    if (argc > 1)
    {
        fptr = fopen(argv[1], "r");
        if (fptr == NULL)
        {
            perror("Error opening file");
        }
    }
    char *paths[MAX_PATHS];
    paths[0] = "/bin/";
    size_t pathsize = 1;
    char *delimiter = " ";
    char *cmd;
    size_t bufsize = MAX_CMD_LEN;
    size_t characters;
    // while ((characters = getline(&cmd, &bufsize, fptr)) != -1)
    // {
    //     if (characters <= 0)
    //     {
    //         printf("End of file reached.\n");
    //         break;
    //     }
    //     printf("%s", cmd);

    //     // Process the command
    //     // (tokenize, execute, etc.)
    // }
    // return 0;
    while (1)
    {
        char *cmd = NULL;
        size_t bufsize = MAX_CMD_LEN;
        size_t characters;
        char *redirection = NULL;

        char *token;

        cmd = (char *)malloc(bufsize * sizeof(char));
        if (cmd == NULL)
        {
            perror("Unable to allocate buffer");
        }
        if (fptr == NULL)
        {
            printf("wish> ");
            characters = getline(&cmd, &bufsize, stdin);
        }
        else
        {
            characters = getline(&cmd, &bufsize, fptr);
            if (characters <= 0)
            {
                break;
                exit(0);
                return 0;
            }
        }
        if (cmd[0] == '#')
        {
            continue;
        }
        // Replace the newline character with null terminator
        for (int i = 0; cmd[i] != '\0'; i++)
        {
            if (cmd[i] == '\t')
                cmd[i] = ' ';
            if (cmd[i] == '\n')
                cmd[i] = '\0';
        }
        token = strsep(&cmd, delimiter);
        const char *cur_cmd = token;
        char *tokens[MAX_TOKENS];
        size_t token_size = 0;

        while (token != NULL && token_size < MAX_TOKENS)
        {
            if (token[0] == '\0')
            {
                token = strsep(&cmd, delimiter);
                continue;
            }
            if (strlen(token) == 1 && token[0] == '>')
            {
                redirection = strsep(&cmd, delimiter);
                printf("%s\n", redirection);
                break;
            }
            tokens[token_size] = token;

            token_size++;
            token = strsep(&cmd, delimiter);
        }
        tokens[token_size] = NULL;
        for (int i = 0; i < token_size; i++)
        {
            printf("%s\n", tokens[i]);
        }

        if (strcmp("exit", cur_cmd) == 0)
        {
            fclose(fptr);
            exit(0);
        }
        else if (strcmp("cd", cur_cmd) == 0)
        {
            if (token_size != 2)
            {
                perror("cd take exactly one argument");
            }
            const char *new_path = tokens[1];
            if (chdir(new_path) != 0)
            {
                perror("change dir failed");
            }
        }
        else if (strcmp("path", cur_cmd) == 0)
        {
            if (token_size == 1)
            {
                perror("specify the path");
                continue;
            }
            size_t path_length = strlen(tokens[1]);
            if (tokens[1][path_length - 1] != '/')
            {
                strcat(tokens[1], "/");
            }
            paths[pathsize] = tokens[1];
            pathsize++;
        }
        else
        {

            int ppid = fork();
            if (ppid < 0)
            {
                // fork failed
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else if (ppid == 0)
            {
                if (redirection != NULL)
                {
                    // 0666 specify file permission
                    int redirection_fd = open(redirection, O_RDWR | O_CREAT, 0666);
                    if (redirection_fd < 0)
                    {
                        perror("cannot open the file");
                    }
                    if (dup2(redirection_fd, STDOUT_FILENO) < 0)
                    {
                        perror("cannot write in this file");
                    }
                    dup2(redirection_fd, STDERR_FILENO);
                    if (close(redirection_fd) < 0)
                    {
                        perror("cannot close the file");
                    }
                }
                for (size_t i = 0; i < pathsize; i++)
                {
                    char full_path[256];         // Allocate a buffer for the full path
                    strcpy(full_path, paths[i]); // Copy path1 to full_path
                    strcat(full_path, cur_cmd);  // Append cmd to full_path
                    // Check if full_path is executable in /bin
                    if (access(full_path, X_OK) != -1)
                    {
                        execv(full_path, tokens);
                        perror("execv");
                    }
                }

                // If no executable found, print error
                perror("command not found");
            }
            else
            {
                wait(NULL);
            }
            free(cmd);
        }
    }
    return 0;
}