#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>

// Struct for each argument in a command
struct commandArg
{
    char *argument;
};

int processCommand(char **commands, char *command)
{
    struct commandArg *curArg = malloc(sizeof(struct commandArg));
    int index = 0;
    int count = 0;

    char *saveptr;

    char *token = strtok_r(command, " ", &saveptr);
    while (token != NULL)
    {
        curArg->argument = calloc(strlen(token) + 1, sizeof(char));
        strcpy(curArg->argument, token);
        commands[index] = token;
        index++;
        count++;

        //     printf("%s\n", curArg->argument);
        //     // curArg->argument = calloc(strlen(token) + 1, sizeof(char));
        //     // curArg->argument = token;
        //     // strcpy(curArg->argument, token);
        //     // tail->next = curArg;
        //     // printf("%s\n", token);
        token = strtok_r(NULL, " ", &saveptr);
    }
    return count;
}

int main(void)
{
    // Boolean for exit command
    bool exitProgram = false;
    // Sting large enough to support 2048 character commands
    char command[2049];
    char *commands[513];
    int test;
    int numArguments;

    do
    {
        // Display colon and flush afterwards
        printf(": ");
        fflush(stdout);
        // Get command that is max input of 2048 characters
        fgets(command, 2048, stdin);
        // If command was exit, set exitProgram boolean to true so that
        // we know to exit the program
        if (strncmp(command, "exit", 4) == 0)
        {
            exitProgram = true;
        }
        else
        {
            numArguments = processCommand(commands, command);
            // Handle blank lines or comments
            if (commands[0][0] == '#' || commands[0][0] == '\n')
            {
                continue;
            }
            else
            {
                printf("this is in 0 %c\n", commands[0][0]);
            }
            // printf("number is: %d", strlen(commands[0]));
        }

    } while (!exitProgram);

    return EXIT_SUCCESS;
}