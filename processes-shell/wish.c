#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "wish.h"

FILE *fptr = NULL;
char *line = NULL;
char *paths[MAX_PATHS] = {"/bin/", NULL};
size_t pathsize = 1;
char *delimiter = " ";

void clean(void)
{
    free(line);
    fclose(fptr);
}
// void *parseInput(void *arg)
// {
// }

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        fptr = fopen(argv[1], "r");
        if (fptr == NULL)
        {
            // perror("Error opening file");
            printError();
        }
    }

    while (1)
    {
        char *line = NULL;
        size_t bufsize = MAX_CMD_LEN;
        size_t characters;
        char *redirection = NULL;

        char *token;

        line = (char *)malloc(bufsize * sizeof(char));
        if (line == NULL)
        {
            perror("Unable to allocate buffer");
        }
        if (fptr == NULL)
        {
            printf("wish> ");
            characters = getline(&line, &bufsize, stdin);
        }
        else
        {
            characters = getline(&line, &bufsize, fptr);
            if (characters == -1)
            {
                // printf("EOF");
                clean();
                exit(0);
                break;
            }
        }
        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }
        for (int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == '\t')
                line[i] = ' ';
            if (line[i] == '\n')
                line[i] = '\0';
            if (line[i] == '>')
            {
            }
        }
        token = strsep(&line, delimiter);
        const char *cur_cmd = token;
        char *tokens[MAX_TOKENS];
        size_t token_size = 0;

        while (token != NULL && token_size < MAX_TOKENS)
        {
            if (token[0] == '\0')
            {
                token = strsep(&line, delimiter);
                continue;
            }
            if (strlen(token) == 1 && token[0] == '>')
            {
                redirection = strsep(&line, delimiter);
                if (redirection == NULL)
                {
                    printError();
                    goto end_loop;
                }
                printf("%s\n", redirection);
                break;
            }
            tokens[token_size] = token;

            token_size++;
            token = strsep(&line, delimiter);
        }
        tokens[token_size] = NULL;
        // for (int i = 0; i < token_size; i++)
        // {
        //     printf("%s\n", tokens[i]);
        // }

        if (strcmp("exit", cur_cmd) == 0)
        {
            if (token_size != 1)
            {
                printError();
                continue;
            }
            clean();
            exit(0);
        }
        else if (strcmp("cd", cur_cmd) == 0)
        {
            if (token_size != 2)
            {
                printError();
                continue;
                // perror("cd take exactly one argument");
            }
            const char *new_path = tokens[1];
            if (chdir(new_path) != 0)
            {
                printError();
                continue;
                // perror("change dir failed");
            }
        }
        else if (strcmp("path", cur_cmd) == 0)
        {
            for (size_t i = 0; i < pathsize; i++)
            {
                paths[i] = NULL;
            }
            pathsize = 0;
            for (size_t i = 1; i < token_size; i++)
            {
                size_t path_length = strlen(tokens[1]);
                if (tokens[i][path_length - 1] != '/')
                {
                    strcat(tokens[i], "/");
                }
                // printf("%s\n", tokens[i]);
                paths[pathsize] = tokens[i];
                pathsize++;
            }
        }
        else
        {

            int ppid = fork();
            if (ppid < 0)
            {
                // fork failed
                // fprintf(stderr, "fork failed\n");
                printError();
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
                        // perror("cannot open the file");
                        printError();
                        continue;
                    }
                    if (dup2(redirection_fd, STDOUT_FILENO) < 0)
                    {
                        // perror("cannot write in this file");
                        printError();
                        continue;
                    }
                    dup2(redirection_fd, STDERR_FILENO);
                    if (close(redirection_fd) < 0)
                    {
                        // perror("cannot close the file");
                        printError();
                        continue;
                    }
                }
                for (size_t i = 0; i < pathsize; i++)
                {
                    char full_path[256];         // Allocate a buffer for the full path
                    strcpy(full_path, paths[i]); // Copy path1 to full_path
                    strcat(full_path, cur_cmd);  // Append line to full_path
                    if (access(full_path, X_OK) != -1)
                    {
                        execv(full_path, tokens);
                    }
                }

                // If no executable found, print error
                printError();
                exit(0);
            }
            else
            {
                wait(NULL);
            }
        end_loop:
            clean();
        }
    }
    return 0;
}