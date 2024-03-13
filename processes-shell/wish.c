#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{

    while (1)
    {
        char *cmd;
        size_t bufsize = 32;
        size_t characters;
        char *path1 = "/bin/";
        char *path2 = "/usr/bin/";

        cmd = (char *)malloc(bufsize * sizeof(char));
        if (cmd == NULL)
        {
            perror("Unable to allocate buffer");
            exit(1);
        }
        printf("wish> ");

        characters = getline(&cmd, &bufsize, stdin);
        // Replace the newline character with null terminator
        if (cmd[characters - 1] == '\n')
        {
            cmd[characters - 1] = '\0';
        }
        printf("%zu characters were read.\n", characters);
        printf("You typed: '%s'\n", cmd);
        if (strcmp("exit", cmd) == 0)
        {
            exit(0);
        }
        // if (strcmp("ls\n", cmd) == 0)
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
                printf("%s", strcat(path1, cmd));
                if (access(strcat(path1, cmd), X_OK) == -1)
                {
                    if (access(strcat(path2, cmd), X_OK) == -1)
                    {
                        perror("no executable file found");
                    }
                    else
                    {
                        execv(strcat(path2, cmd), (char *[]){cmd, NULL});
                    }
                }
                else
                {
                    execv(strcat(path1, cmd), (char *[]){cmd, NULL});
                }
                // child (new process)
                // printf("child (pid:%d)\n", (int)getpid());
                // When calling execv, it replaces the current process with a new process
                // If the execv call is successful, it doesn't return to the original code of the child process
                // So, any code after execv call will not be executed unless execv fails.
                // execv("/bin/ls", (char *[]){"ls", NULL});
                // printf("child end\n");
            }
            else
            {
                // Wait for the child to finish
                // printf("parent start\n");
                wait(NULL);
                // printf("parent end\n");
            }
        }
    }

    return 0;
}