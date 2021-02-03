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
        //     // curArg->argument = token;
        token = strtok_r(NULL, " ", &saveptr);
    }
    return count;
}

void replaceString(char **commands, int start, int index)
{
    // Convert pid to string. Set to 8 for max pid size of 64-bit plus null
    char pid[8];
    sprintf(pid, "%d", getpid());
    // printf("%s\n", pid);
    // printf("%d\n", strlen(pid));
    // char newCommand[strlen(commands[index]) + strlen(pid)];
    char *newCommand = calloc(strlen(commands[index]) + strlen(pid) + 1, sizeof(char));
    int newIndex;
    int i;
    // printf("%s\n", newCommand);
    for (newIndex = 0; newIndex < start; newIndex++)
    {
        newCommand[newIndex] = commands[index][newIndex];
        // printf("%c\n", commands[index][newIndex]);
    }
    // newCommand[newIndex] = 0;
    strcat(newCommand, pid);
    char *finalPart = calloc(strlen(commands[index]) + 1, sizeof(char));
    // char finalPart[strlen(commands[index])];
    newIndex = 0;
    for (i = start + 2; i < strlen(commands[index]); i++)
    {
        // printf("%c\n", commands[index][i]);

        finalPart[newIndex] = commands[index][i];
        newIndex++;
    }
    strcat(newCommand, finalPart);
    commands[index] = newCommand;
}

void variableExpansion(char **commands, int numArguments)
{
    int index = 0;     // pointer to command in commands array
    bool didExpansion; // bool to see if we need to check for another expansion
    int i;             // pointer to characters of each command (nested for loop)

    while (index < numArguments)
    {
        didExpansion = false;
        for (i = 0; i < strlen(commands[index]) - 1; i++)
        {
            // printf("%c\n", commands[index][i]);
            // Check if we have two '$' in a row and variable expansion is necessary
            if (commands[index][i] == '$')
            {
                if (commands[index][i + 1] == '$')
                {
                    // printf("Changing: %s\n", commands[index]);
                    replaceString(commands, i, index);
                    didExpansion = true;
                    break;
                }
            }
        }

        // We only want to progress pointer if we did not do an expansion
        // otherwise, we want to check that same command again to see
        // if it needs more expansions
        if (!didExpansion)
        {
            index++;
        }
        fflush(stdout);
    }
    index = 0;
    while (index < numArguments)
    {
        printf("%s\n", commands[index]);
        index++;
    }
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
        if (strncmp(command, "exit", 4) == 0 && strlen(command) == 5)
        {
            exitProgram = true;
        }
        else
        {
            numArguments = processCommand(commands, command);
            // Handle blank lines or comments by checking if first char
            // is '#' or a blank line, i.e. '\n'
            if (commands[0][0] == '#' || commands[0][0] == '\n')
            {
                continue;
            }
            else
            {
                // printf("this is in 0 %c\n", commands[0][0]);
                // printf("The pid is: %d\n", getpid());
                // handle neccessary variable expansion
                variableExpansion(commands, numArguments);
            }
            // printf("number is: %d", strlen(commands[0]));
        }

    } while (!exitProgram);

    return EXIT_SUCCESS;
}