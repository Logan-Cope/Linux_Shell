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
    char *program;
    char *arguments[512];
    char *tokens[512];
    int tokenCount;
};

int status = 0;

/* This function tokenizes the user entered command and
stores each command as a strcut commandArg in an array.
It takes, the commands array and the command as parameters
and returns an integer represeting the number of arguments
in the given command
*/
struct commandArg *processCommand(char *command)
{
    // dynamically create commandArg struct
    struct commandArg *cmd = malloc(sizeof(struct commandArg));

    int count = 0; // keeps track of number of elements in the array

    // For use with strtok_r
    char *token;
    char *rest = command;
    memset(cmd->tokens, 0, sizeof(cmd->tokens));
    token = strtok_r(rest, " ", &rest);

    int index = 0; // keeps track of which index in the array we are on

    while (token != NULL)
    {
        cmd->tokens[index] = token;
        token = strtok_r(NULL, " ", &rest);
        index++;
    }
    int i;
    // for (i = 0; i < index; i++)
    // {
    //     printf("%s\n", cmd->tokens[i]);
    // }
    cmd->tokenCount = index;
    // printf("%d\n", cmd->tokenCount);
    return cmd;
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
        if (chdir(getenv("HOME")) == -1)
        {
            printf("there was an err\n");
        }
    }
    else
    {
        if (chdir(commands[1]) == -1)
        {
            printf("there was an err\n");
        }
    }
}

void parseCommands(struct commandArg *cmd)
{
    int i;         // used to loop through tokens
    int index = 0; // used to fill in arguments
    for (i = 0; i < cmd->tokenCount; i++)
    {
        // Do not include final & in cmd->arguments because it's only
        // indicating to run it in the background but we don't want
        // to pass it to exec
        if (i == cmd->tokenCount - 1 && strcmp(cmd->tokens[i], "&") == 0)
        {
            continue;
        }
        // Do not include < or >in cmd->arguments because these is
        // for redirection but we don't want to pass them to exec
        if (strcmp(cmd->tokens[i], "<") == 0 || strcmp(cmd->tokens[i], ">") == 0)
        {
            i++;
            continue;
        }
        // Otherwise, copy the raw token into the arguments array
        else
        {
            cmd->arguments[index] = cmd->tokens[i];
            index++;
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
        for (i = 0; i < 513; i++)
        {
            commands[i] = NULL;
        }
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
            // Remove \n from the end so that the string will continue terminating
            // on \0 instead.
            strtok(command, "\n");
            struct commandArg *cmd = processCommand(command);
            numArguments = cmd->tokenCount;
            // Handle neccessary variable expansion
            variableExpansion(cmd->tokens, numArguments);
            // Remove commands with < or >
            parseCommands(cmd);

            // Handle blank lines or comments by checking if first char
            // is '#' or a blank line, i.e. '\n'
            if (cmd->tokens[0][0] == '#' || cmd->tokens[0][0] == '\n')
            {
                continue;
            }
            else
            {
                // Check if we need to cd
                if (strncmp(cmd->tokens[0], "cd", 2) == 0 && strlen(cmd->tokens[0]) == 2)
                {
                    changeDirectory(cmd->tokens, numArguments);
                }

                // Check if we need to print the status
                if (strcmp(cmd->tokens[0], "status") == 0)
                {
                    printf("%d\n", status);
                }

                else
                {
                    int redirectOut;
                    bool redirectedOut = false;
                    int redirectIn;
                    bool redirectedIn = false;
                    for (i = 0; i < numArguments; i++)
                    {
                        if (strncmp(cmd->tokens[i], ">", 2) == 0 && strlen(cmd->tokens[i]) == 1)
                        {
                            // printf("%s\n", commands[i + 1]);
                            redirectedOut = true;
                            redirectOut = i;
                        }
                        if (strncmp(cmd->tokens[i], "<", 2) == 0 && strlen(cmd->tokens[i]) == 1)
                        {
                            redirectedIn = true;
                            redirectIn = i;
                        }
                    }
                    int targetFD;
                    int sourceFD;

                    pid_t spawnid = -5;
                    int childStatus;
                    int childPid;
                    spawnid = fork();

                    if (spawnid == -1)
                    {
                        perror("Falied to fork\n");
                        fflush(stdout);
                        exit(1);
                        // printf("Falied to fork");
                    }
                    else if (spawnid == 0)
                    {
                        if (redirectedOut)
                        {
                            targetFD = open(cmd->tokens[redirectOut + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (targetFD == -1)
                            {
                                printf("Output file did not open");
                                fflush(stdout);
                            }
                            else
                            {
                                dup2(targetFD, 1);
                            }
                        }
                        if (redirectedIn)
                        {
                            // printf("This is the file to open: %s\n", commands[redirectIn - 1]);
                            sourceFD = open(cmd->tokens[redirectIn + 1], O_RDONLY);
                            if (sourceFD == -1)
                            {
                                printf("Input file did not open: %s\n", cmd->tokens[redirectIn + 1]);
                                fflush(stdout);
                            }
                            // printf("here is: %s\n", cmd->tokens[redirectIn + 1]);
                            dup2(sourceFD, 0);
                            cmd->tokens[redirectIn] = NULL;
                        }

                        // Execute other commands
                        execvp(cmd->arguments[0], cmd->arguments);

                        printf("Execvp didn't work\n");
                    }
                    else
                    {
                        childPid = waitpid(-1, &childStatus, 0);
                        // Check if process exited normally and set status
                        if (WIFEXITED(childStatus))
                        {
                            printf("Process exited normally\n");
                            status = WEXITSTATUS(childStatus);
                            // printf("%d\n", status);
                        }
                        // Check if process was terminated by a signal and set status
                        else if (WIFSIGNALED(childStatus) != 0)
                        {
                            printf("Process was terminated by a singal\n");
                            status = WTERMSIG(childStatus);
                        }
                        // // Close files
                        // if (redirectedOut)
                        // {
                        //     close(targetFD);
                        // }
                        // if (redirectedIn)
                        // {
                        //     close(sourceFD);
                        // }

                        // printf("%s\n", childStatus);
                        // status = childStatus;
                    }
                }
            }
        }

    } while (!exitProgram);

    return EXIT_SUCCESS;
}