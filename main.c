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

/* This function tokenizes the user entered command and
stores each command as a strcut commandArg in an array.
It takes, the commands array and the command as parameters
and returns an integer represeting the number of arguments
in the given command
*/
int processCommand(char **commands, char *command)
{
    // dynamically create commandArg struct
    struct commandArg *curArg = malloc(sizeof(struct commandArg));
    int index = 0; // keeps track of which index in the array we are on
    int count = 0; // keeps track of number of elements in the array

    // For use with strtok_r
    char *saveptr;

    // Get the first token
    char *token = strtok_r(command, " ", &saveptr);

    // Continue to tokenize until we have stored all the arguments
    while (token != NULL)
    {
        // Store the argument in the commands array
        curArg->argument = calloc(strlen(token) + 1, sizeof(char));
        strcpy(curArg->argument, token);
        commands[index] = token;
        // Increment both of our counters
        index++;
        count++;
        // Get next argument to be stored
        token = strtok_r(NULL, " ", &saveptr);
    }
    // Return the number of our arguments in our array.
    // Note: I could have just returned index, but I made a
    // design decision to specify a count that's only has this
    // purpose
    return count;
}

/* This function is used to replace an argument with an
a variable expanded argument and update the commands array
to hold this new argument.
It takes, the commands array, the index in which the first
'$' that needs to be replaced occurs, and the index of
which argument in the command array needs to be updated
as parameters
*/
void replaceString(char **commands, int start, int index)
{
    // Convert pid to string. Set to 8 for max pid size of 64-bit plus null
    char pid[8];
    sprintf(pid, "%d", getpid());

    // Initialize a new string to hold the replacement
    char *newCommand = calloc(strlen(commands[index]) + strlen(pid) + 1, sizeof(char));
    int newIndex; // used in for loop to fill in beginning of argument up to '$'
    int i;

    // Fill the new string with the characters of the original string
    // up to and not including the '$'
    for (newIndex = 0; newIndex < start; newIndex++)
    {
        newCommand[newIndex] = commands[index][newIndex];
    }

    // Concatenate the pid onto the new string
    strcat(newCommand, pid);

    // Fill in the remaining characters starting with the character after
    // the 2nd '$'. This will allow us to effectively replace $$ with pid
    char *finalPart = calloc(strlen(commands[index]) + 1, sizeof(char));
    newIndex = 0;
    for (i = start + 2; i < strlen(commands[index]); i++)
    {
        finalPart[newIndex] = commands[index][i];
        newIndex++;
    }
    // Concatenate this last part of the string onto our new command
    strcat(newCommand, finalPart);
    // Update the array with this now variable expanded command
    commands[index] = newCommand;
}

/* This function is used to perform variable expansions
when necessary. It takes, the commands array and the number
of arguments in the command as parameters
*/
void variableExpansion(char **commands, int numArguments)
{
    int index = 0;     // pointer to command in commands array
    bool didExpansion; // bool to see if we need to check for another expansion
    int i;             // pointer to characters of each command (nested for loop)

    // Loop through all arguments checking for the need to expand variables
    while (index < numArguments)
    {
        // Set didExpansion to false so that if we are on an argument that does
        // not need an expansion, we know to move to the next argument
        didExpansion = false;
        for (i = 0; i < strlen(commands[index]) - 1; i++)
        {
            // Check if we have two '$' in a row and variable expansion is necessary
            if (commands[index][i] == '$')
            {
                if (commands[index][i + 1] == '$')
                {
                    // Send it to replaceString to be expanded
                    replaceString(commands, i, index);
                    // Set didExpansion to true and we will check this again
                    // to see if further expansions are necessary
                    didExpansion = true;
                    // Break out, we will check this again on the next iteration
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
    // index = 0;
    // while (index < numArguments)
    // {
    //     printf("%s\n", commands[index]);
    //     index++;
    // }
}

/* This function is used to change the directory. It takes,
the commands array and the number of arguments in the
command as parameters
*/
void changeDirectory(char **commands, int numArguments)
{
    if (numArguments == 1)
    {
        printf("Home\n");
        if (chdir(getenv("HOME")) == -1)
        {
            printf("there was an err\n");
        }
    }
    else
    {
        printf("not home\n");
        if (chdir(commands[1]) == -1)
        {
            printf("there was an err\n");
        }
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
    int i;
    for (i = 0; i < 513; i++)
    {
        commands[i] = NULL;
    }

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

            strtok(command, "\n");
            numArguments = processCommand(commands, command);

            // for (i = 0; i < 10; i++)
            // {
            //     printf("%s\n", commands[i]);
            // }
            // Handle blank lines or comments by checking if first char
            // is '#' or a blank line, i.e. '\n'
            if (commands[0][0] == '#' || commands[0][0] == '\n')
            {
                continue;
            }
            else
            {

                // Remove \n from the end so that the string will continue terminating
                // on \0 instead.
                // strtok(command, "\n");
                // Handle neccessary variable expansion
                variableExpansion(commands, numArguments);

                // Check if we need to cd
                if (strncmp(commands[0], "cd", 2) == 0 && strlen(commands[0]) == 2)
                {
                    changeDirectory(commands, numArguments);
                }
                // else if (strncmp(commands[0], "ls", 2) == 0 && strlen(commands[0]) == 2)
                else
                {
                    // char *newargv[] = {commands[0], commands[1]};
                    // execvp("ls", newargv);
                    // execvp(commands[0], commands);

                    // int i;
                    // for (i = 0; i < numArguments; i++)
                    // {
                    //     execvp(commands[i], commands);
                    // }

                    pid_t spawnid = -5;
                    int childStatus;
                    int childPid;
                    spawnid = fork();
                    if (spawnid == -1)
                    {
                        printf("this didn't work");
                    }
                    else if (spawnid == 0)
                    {
                        execvp(commands[0], commands);
                        // printf("I am a child");
                    }
                    else
                    {
                        childPid = waitpid(-1, &childStatus, 0);
                        // printf("parent is waiting");
                    }
                }
            }
        }

    } while (!exitProgram);

    return EXIT_SUCCESS;
}