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
        char *token;
        char *delimiter = " ";

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
            cmd[characters - 1] = '\0';
        if (strcmp("exit", cmd) == 0)
        {
            exit(0);
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
                token = strsep(&cmd, delimiter);
                char *argv[10];
                size_t i = 0;
                while (token != NULL)
                {
                    argv[i] = token;
                    i++;
                    token = strsep(&cmd, delimiter);
                }
                argv[i] = NULL;

                char full_path[256];        // Allocate a buffer for the full path
                strcpy(full_path, path1);   // Copy path1 to full_path
                strcat(full_path, argv[0]); // Append cmd to full_path
                // Check if full_path is executable in /bin
                if (access(full_path, X_OK) != -1)
                {
                    execv(full_path, argv);
                    perror("execv");
                }

                strcpy(full_path, path2);   // Copy path2 to full_path
                strcat(full_path, argv[0]); // Append cmd to full_path

                // Check if full_path is executable in /usr/bin
                if (access(full_path, X_OK) != -1)
                {
                    execv(full_path, argv);
                    // execv(full_path, (char *[]){cmd, NULL});
                    perror("execv");
                }

                // If no executable found, print error
                perror("command not found");
            }
            else
            {
                wait(NULL);
            }
        }
    }

    return 0;
}