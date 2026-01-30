#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>

int MAX_ARGS_SIZE = 3;

int badcommand()
{
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist()
{
    printf("Bad command: File not found\n");
    return 3;
}
int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo(char *token);
int my_ls();
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *dirname);
int run(char *command_args[], int args_size);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size)
{
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE)
    {
        return badcommand();
    }

    for (i = 0; i < args_size; i++)
    { // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0)
    {
        // help
        if (args_size != 1)
            return badcommand();
        return help();
    }
    else if (strcmp(command_args[0], "quit") == 0)
    {
        // quit
        if (args_size != 1)
            return badcommand();
        return quit();
    }
    else if (strcmp(command_args[0], "set") == 0)
    {
        // set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);
    }
    else if (strcmp(command_args[0], "print") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);
    }
    else if (strcmp(command_args[0], "source") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);
    }
    else if (strcmp(command_args[0], "echo") == 0)
    {
        // echo takes exactly 1 argument (the string or $var)
        // total size is 2 (command + arg)
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);
    }
    else if (strcmp(command_args[0], "my_ls") == 0)
    {
        if (args_size != 1)
            return badcommand();
        return my_ls();
    }
    else if (strcmp(command_args[0], "my_mkdir") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);
    }
    else if (strcmp(command_args[0], "my_touch") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_touch(command_args[1]);
    }
    else if (strcmp(command_args[0], "my_cd") == 0)
    {
        if (args_size != 2)
            return badcommand();
        return my_cd(command_args[1]);

        // ...
    }
    else if (strcmp(command_args[0], "run") == 0)
    {
        return run(command_args, args_size);
    }
    else
        return badcommand();
}

// commands and their helpers
int help()
{

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit()
{
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value)
{
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}

int print(char *var)
{
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script)
{
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt"); // the program is in a file

    if (p == NULL)
    {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1)
    {
        errCode = parseInput(line); // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p))
        {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *token)
{
    // Case 1: Variable (starts with $)
    if (token[0] == '$')
    {
        // (token + 1) to skip the '$' symbol and get the variable name
        char *value = mem_get_value(token + 1);

        // Check if the variable exists
        if (strcmp(value, "Variable does not exist") == 0)
        {
            printf("\n");
        }
        else
        {
            printf("%s\n", value);
            free(value); // used strdup, so we must free
        }
    }
    // Case 2: Literal string
    else
    {
        printf("%s\n", token);
    }
    return 0;
}

int compare(const void *a, const void *b)
{
    // comparision function for qsort to sort filenames
    const char *pa = *(const char **)a;
    const char *pb = *(const char **)b;
    return strcmp(pa, pb);
}

int my_ls()
{
    struct dirent *de;
    DIR *dr = opendir(".");

    if (dr == NULL)
    {
        printf("Could not open current directory\n");
        return 0;
    }

    // Store filenames in an array for sorting
    // Assuming max 100 files, max length 100 (from constraints)
    char *fileList[100];
    int count = 0;

    while ((de = readdir(dr)) != NULL)
    {
        if (count < 100)
        {
            fileList[count] = strdup(de->d_name);
            count++;
        }
    }
    closedir(dr);

    // Sorting the array (Numbers > Uppercase > Lowercase)
    qsort(fileList, count, sizeof(char *), compare);

    // Print and clean up
    for (int i = 0; i < count; i++)
    {
        printf("%s\n", fileList[i]);
        free(fileList[i]);
    }

    return 0;
}

int is_valid_name(char *name)
{
    // check strictly for alphanumeric (no underscores/symbols)
    for (int i = 0; name[i] != '\0'; i++)
    {
        if (!isalnum(name[i]))
        {
            return 0; // non-alphanumeric character found
        }
    }
    return 1;
}

int my_mkdir(char *dirname)
{
    char *folderName = dirname;
    int isVar = 0;

    // Handle variable substitution ($var)
    if (dirname[0] == '$')
    {
        char *value = mem_get_value(dirname + 1);
        if (strcmp(value, "Variable does not exist") == 0)
        {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        folderName = value;
        isVar = 1;
    }

    // Checking if the resolved name is strictly alphanumeric
    if (!is_valid_name(folderName))
    {
        printf("Bad command: my_mkdir\n");
        if (isVar)
            free(folderName);
        return 1;
    }

    int result = mkdir(folderName, 0777);

    if (isVar)
        free(folderName);

    return 0;
}

int my_touch(char *filename)
{
    FILE *fp = fopen(filename, "w"); // create an empty file
    // check if file creation was successful
    if (fp != NULL)
    {
        fclose(fp);
    }
    else
    {
        printf("Could not create file\n");
    }
    return 0;
}

int my_cd(char *dirname)
{
    // changes the name and check if dirname doesn not exist inside current directory
    if (chdir(dirname) != 0)
    {
        printf("Bad command: my_cd\n");
    }
    return 0;
}

int run(char *command_args[], int args_size)
{
    if (args_size < 2)
        return badcommand();

    fflush(stdout); // Force the "Shell version..." line to print NOW

    pid_t pid = fork();

    if (pid < 0)
    {
        // Fork failed
        return 1;
    }
    else if (pid)
    {
        // Parent process
        int status;
        // Wait for the specific child process to finish
        waitpid(pid, &status, 0);
    }
    else
    {
        // Child process
        // execvp expects a NULL-terminated array of strings.
        command_args[args_size] = NULL;
        // execution of commands
        execvp(command_args[1], &command_args[1]);

        // If execvp returns, it means there was an error (e.g. command not found)
        exit(1);
    }

    return 0;
}