/* ICCS227: Project 1: icsh
 * Name: Natthapas Ngamsukhonratana
 * StudentID: 6581155
 */

// #include "ProcessTracker.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_CMD_BUFFER 255

pid_t suspended_pid = -1;
pid_t child_pid;
pid_t mainBranch;

int processMap[1000][3]; //Keeps a uh { {Order, PID, isBackground }, ... } pair.
int size = 1;

int GetMapID(pid_t processID){
    int processOrder;
    for (int i = 0; i < sizeof(processMap); i++){
        if (processMap[i][1] == processID){
            processOrder = processMap[i][0];
        }
    }
    return processOrder;
}

void addPIDMap(pid_t processID){
    processMap[size-1][0] = size;
    processMap[size-1][1] = processID;
    processMap[size-1][2] = 1; //is forground process.
    size++;
}

int removePIDMap(pid_t processID){
    for (int i = 0;i < 1000; i++){
        if (processMap[i][1] == processID){
            size--;
            return processMap[i][0];
        }
        else if (processMap[i][0] == 0 && processMap[i][1] == 0){
            break;
        }
    }
    size--;
    return -1; // Process not found, broken.
}

int getProcess(pid_t processID){
    for (int i = 0;i < 1000; i++){
        if (processMap[i][1] == processID){
            size--;
            return processMap[i][2];
        }
        else if (processMap[i][0] == 0 && processMap[i][1] == 0 && processMap[i][2] == 0){
            break;
        }
    }
    size--;
    return -1; // Process not found, broken.
}

void output(char fileName[])
{ // This function redirects the stdout to the file.
    int file_desc = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(file_desc, STDOUT_FILENO); // This makes stdout the file itself.
    close(file_desc);
}

void input(char fileName[])
{ // Redirect the file to make it stdin.
    printf("%s\n", fileName);
    int file_desc = open(fileName, O_RDONLY, 0777); // Points STDIN to the file.
    dup2(file_desc, STDIN_FILENO);
    close(file_desc);
}

int outsideProcess(char *instruct)
{ // Pass in Commands that are already splitted
    int resetSTDOUT = dup(STDOUT_FILENO);
    int resetSTDIN = dup(STDIN_FILENO);

    int isForeground = 0;
    char *foregroundProcesses = strchr(instruct, '&');
    if (foregroundProcesses != NULL)
    {
        instruct = strtok(instruct, "&");
        isForeground = 1;
    }

    char *redir = strchr(instruct, '>'); // Getting the fileName
    char file[255];
    char *arr = file;
    int indexOut; // no redirection
    if (redir != NULL)
    { // is found.
        indexOut = 0;
        while (instruct[indexOut] != '>')
        {
            indexOut++;
        }
        for (int i = indexOut; i < strlen(instruct); i++)
        {
            file[i - indexOut] = instruct[i];
        }
        strtok(file, " ");
        arr = strtok(NULL, " ");
        instruct = strtok(instruct, ">");
        output(arr); // redirect output to the file.
    }

    redir = strchr(instruct, '<'); // Getting the fileName
    if (redir != NULL)
    {
        indexOut = 0;
        while (instruct[indexOut] != '<')
        {
            indexOut++;
        }
        for (int i = indexOut; i < strlen(instruct); i++)
        {
            file[i - indexOut] = instruct[i];
        }
        strtok(file, " ");
        arr = strtok(NULL, " ");
        instruct = strtok(instruct, "<");
        input(arr);
        printf("%s\n", arr);
    }

    char *prog_arv[255];
    char *command = strtok(instruct, " "); // First part of the instruct, command part.

    char str[100] = "/usr/bin/";
    prog_arv[0] = strcat(str, command);

    int index = 1; // Parsing command.
    while (command != NULL)
    {
        command = strtok(NULL, " ");
        prog_arv[index] = command;
        index++;
    }
    signal(SIGTSTP, SIG_IGN); // ignore crtlz

    child_pid = fork(); // 0 in the child itself but sth else outside.
    addPIDMap(child_pid);
    if (child_pid == 0)
    {
        // child process
        signal(SIGINT, SIG_DFL); // specify default signal action.
        signal(SIGTSTP, SIG_DFL);

        prog_arv[index] = NULL;
        suspended_pid = getpid();
        int err = execvp(prog_arv[0], prog_arv); // will run the whole list
        if (err == -1)
        {
            return -1;
        }
        // this replace the memory iff it is successful
    }

    int status;
    if (isForeground != 1)
    {
        waitpid(child_pid, &status, WUNTRACED);
    }
    else if (isForeground == 1)
    {
        printf("%d is running.\n", child_pid);
    }

    
    dup2(resetSTDOUT, STDOUT_FILENO); // sets stdout back to terminal.
    dup2(resetSTDIN, STDIN_FILENO);

    // 3 cases
    // 1 is if program dne, 2 ping is found but error
    if (WIFEXITED(status)){ // if true then it means it exited normally
        int statusCode = WEXITSTATUS(status);
        if (statusCode == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    return 0;
    // main process
}

void echo(char *instruct){ // Printing stuff
    for (int i = 5; i < strlen(instruct); i++)
    {
        printf("%c", instruct[i]);
    }
    printf("\n");
}

int checkCm(char *commands){ // 0 is current com, 1 is prev
    if (strncmp(commands, "echo ", 5) == 0){
        echo(commands);
        return 0;
    }
    else{
        int isExist = outsideProcess(commands);
        if (isExist == 0){ // does exist
            return 0;
        }
    }
    return -1; // command does not exist
}

void childHandler(int sig)
{   
    int status;
    pid_t deadChild = waitpid(-1, &status, WNOHANG);
    int isBack = getProcess(deadChild);

    if (isBack != 1){
        return;
    }

    int pidOrder = removePIDMap(deadChild);
    printf("\n[%d] %d has died.\n", pidOrder, deadChild);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    char buffer[MAX_CMD_BUFFER];
    char instruct[MAX_CMD_BUFFER];
    char prevInstruct[MAX_CMD_BUFFER];
    char temp[MAX_CMD_BUFFER];

    char *Instructions[2] = {NULL, NULL};
    int mode;
    uint8_t exit = 0;

    mainBranch = getpid();
    signal(SIGINT, SIG_IGN); // Ignore signal
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, childHandler);

    if (argc > 1)
    { // script mode
        FILE *fptr = fopen(argv[1], "r");
        while (fgets(buffer, sizeof(buffer), fptr))
        {
            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(instruct, buffer);

            Instructions[0] = instruct;
            Instructions[1] = prevInstruct;

            if (strncmp(Instructions[0], "exit", 4) == 0)
            {   
                exit = atoi(strncpy(temp, instruct + 4, 251));
                printf("bye lol\n");
                break;
            }

            if (strncmp(Instructions[0], "fg", 2) == 0)
            {
                kill(suspended_pid, SIGCONT);
                continue;
            }

            if (strncmp(Instructions[0], "!!", 2) == 0)
            {
                mode = checkCm(Instructions[1]); // get like !!
                continue;
            }
            else
            {
                mode = checkCm(Instructions[0]); // Check the commands and runs it
            }

            // Operations for dealing with cmd list.
            if (mode == 1)
            { // case where copy does not req.
                continue;
            }
            else if (mode == 0)
            { // Normal case
                strcpy(prevInstruct, instruct);
            }
            else if (mode == -1)
            {
                printf("Command does not exist u dumb >:(\n");
                continue;
            }
        }
        fclose(fptr);
        return (uint8_t)exit;
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpid());

    while (1)
    { // Normal mode, user input thing

        fflush(stdin);
        printf("icsh $ ");

        fgets(buffer, 255, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "") == 0)
        {
            continue;
        }

        strcpy(instruct, buffer);

        Instructions[0] = instruct;
        Instructions[1] = prevInstruct;


        if (strncmp(Instructions[0], "exit ", 5) == 0)
        {
            exit = atoi(strncpy(temp, instruct + 4, 251));
            printf("bye lol\n");
            break;
        }

        if (strncmp(Instructions[0], "fg", 2) == 0)
        {

            tcsetpgrp(STDIN_FILENO, child_pid);
            kill(child_pid, SIGCONT);

            int status;
            waitpid(child_pid, &status, WUNTRACED);
            continue;
        }

        if (strncmp(Instructions[0], "!!", 2) == 0)
        {
            mode = checkCm(Instructions[1]); // get like !!
            continue;
        }
        else
        {
            mode = checkCm(Instructions[0]); // Check the commands and runs it
        }

        // Operations for dealing with cmd list.
        if (mode == 1)
        { // case where cpy is not req.
            continue;
        }
        else if (mode == 0)
        { // Normal case
            strcpy(prevInstruct, instruct);
        }
        else if (mode == -1)
        {
            printf("Command does not exist u dumb >:(\n");
        }
    }

    return (uint8_t)exit;
}
