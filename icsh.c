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

// pid_t suspended_pid = -1;
pid_t child_pid;
pid_t mainBranch;
pid_t fg_pgid;

//Start of some random DS #######################################################

int processMap[1000][3]; //Keeps a uh { {Order, PID, isBackground }, ... } pair.
int size = 1;

int GetMapID(pid_t processID){ // this works.
    int processOrder;
    for (int i = 0; i < size; i++){
        if (processMap[i][1] == processID){
            processOrder = processMap[i][0];
        }
    }
    return processOrder;
}

void addPIDMap(pid_t processID, int isForeground){ // adding stuff
    processMap[size-1][0] = size;
    processMap[size-1][1] = processID;
    processMap[size-1][2] = isForeground; //is forground process.
    size++;
}

int removePIDMap(pid_t processID){ // return the number process.
    int retVal;
    for (int i = 0;i < size; i++){
        if (processMap[i][1] == processID){
            retVal = processMap[i][0];
            for (int j = i; j < size-1; j++){
                processMap[j][0] = processMap[j+1][0];
                processMap[j][1] = processMap[j+1][1];
                processMap[j][2] = processMap[j+1][2];
             }
             size--;
             return retVal;
        }
    }
    return -1; // Process not found, broken.
}

void printProcess(){
    for (int i = 0; i<size-1; i++){
        printf("[%d] Running\n", processMap[i][0]);
    }
}

void bringUp(char* instruct){
    signal(SIGTTOU, SIG_IGN);
    strtok(instruct, " ");
    char* processNum = strtok(NULL, " ");
    int pNum = atoi(processNum);
    printf("%d", processMap[pNum-1][1]);
    tcsetpgrp(STDIN_FILENO, processMap[pNum-1][1]);
}

void output(char fileName[]){ // This function redirects the stdout to the file.
    int file_desc = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(file_desc, STDOUT_FILENO); // This makes stdout the file itself.
    close(file_desc);
}

void input(char fileName[]){ // Redirect the file to make it stdin.
    printf("%s\n", fileName);
    int file_desc = open(fileName, O_RDONLY, 0777); // Points STDIN to the file.
    dup2(file_desc, STDIN_FILENO);
    close(file_desc);
}

//END of DS ###########################################



// Processes ########## This part is so shit #######################

int outsideProcess(char *instruct){ // Pass in Commands that are already splitted
    int resetSTDOUT = dup(STDOUT_FILENO); // to reset the io
    int resetSTDIN = dup(STDIN_FILENO);

    
    //*file redirection part */
    int isForeground = 0; // Seperating string process.
    char *foregroundProcesses = strchr(instruct, '&');
    if (foregroundProcesses != NULL){
        instruct = strtok(instruct, "&");
        isForeground = 1; // is not foreground process.
    }

    char *redir = strchr(instruct, '>'); // Getting the fileName
    char file[255];
    char *arr = file;
    int indexOut; // no redirection
    if (redir != NULL){ // is found.
        indexOut = 0;
        while (instruct[indexOut] != '>'){
            indexOut++;
        }
        for (int i = indexOut; i < strlen(instruct); i++){
            file[i - indexOut] = instruct[i];
        }
        strtok(file, " ");
        arr = strtok(NULL, " ");
        instruct = strtok(instruct, ">");
        output(arr); // redirect output to the file.
    }

    redir = strchr(instruct, '<'); // Getting the fileName
    if (redir != NULL){
        indexOut = 0;
        while (instruct[indexOut] != '<'){
            indexOut++;
        }
        for (int i = indexOut; i < strlen(instruct); i++){
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
    //*done with file parsing part */


    signal(SIGINT, SIG_IGN); // ignore ctrlc    
    signal(SIGTSTP, SIG_IGN); // ignore crtlz


    child_pid = fork(); // 0 in the child itself but sth else outside.
    addPIDMap(child_pid, isForeground); // 1 is sent to fg immediately, 0 stays outside
    // fflush(stdout);
    if (child_pid == 0){
        // fflush(stdout);

        // child process
        signal(SIGTTOU, SIG_IGN);
        signal(SIGINT, SIG_DFL); // specify default signal action.
        signal(SIGTSTP, SIG_DFL);

        setpgid(0, 0); // set group id to its own uniqe group setpgid(getpid(), getpid())
        
        prog_arv[index] = NULL; // command thing.
        int err = execvp(prog_arv[0], prog_arv); // will run the whole list
        fflush(stdout);
        if (err == -1){
            return -1;
        }
        // this replace the memory iff it is successful
    }

    // fflush(stdout);
    setpgid(child_pid, child_pid);
    int status;
    if (isForeground == 0){
        tcsetpgrp(STDIN_FILENO, child_pid);
        waitpid(child_pid, &status, WUNTRACED);
        // fflush(stdout);
        removePIDMap(child_pid);
        tcsetpgrp(STDIN_FILENO, getpid());
    }
    else if (isForeground == 1){
        printf("%d is running.\n", child_pid);
    }


    // fflush(stdout);
    dup2(resetSTDOUT, STDOUT_FILENO); // sets stdout back to terminal.
    dup2(resetSTDIN, STDIN_FILENO);
    
    // fflush(stdout);

    // removePIDMap(child_pid);

    // 3 cases
    // 1 is if program dne, 2 ping is found but error
    if (WIFEXITED(status)){ // if true then it means it exited normally
        int statusCode = WEXITSTATUS(status);
        if (statusCode == 0){
            return 0;
        }
        else{
            return -1;
        }
    }
    return 0;
    // main process
}

// End of Shitty Processes ########################################################



void echo(char *instruct){ // Printing stuff
    for (int i = 5; i < strlen(instruct); i++){
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




//################################################
//Handler ******

void childHandler(int sig){   // Still fucks up my format part. 

    fflush(stdout);

    int status;
    pid_t deadChild;
    
    char msg[MAX_CMD_BUFFER];
    
    while ((deadChild = waitpid(-1, &status, WNOHANG)) > 0){ //
        fflush(stdout);
        int pidOrder = removePIDMap(deadChild);
        int len = snprintf(msg, sizeof(msg), "\nChild [%d] %d exited.\nicsh $ ",pidOrder, deadChild);
        fflush(stdout);
        fflush(stdin);
        write(STDOUT_FILENO, msg, len);
        fflush(stdout);
    }
    fflush(stdout);
    
}

void handleTSTP(int sig){
    fflush(stdout);
    kill(-child_pid, SIGSTOP);
    fflush(stdout);
    tcsetpgrp(STDIN_FILENO, getpid());
    fflush(stdout);
}


// END of Handler
//####################################################


int main(int argc, char *argv[]){
    char buffer[MAX_CMD_BUFFER];
    char instruct[MAX_CMD_BUFFER];
    char prevInstruct[MAX_CMD_BUFFER];
    char temp[MAX_CMD_BUFFER];

    char *Instructions[2] = {NULL, NULL};
    int mode = 0;
    uint8_t exit = 0;

    mainBranch = getpid();
    signal(SIGINT, SIG_IGN); // Ignore signal
    signal(SIGTSTP, handleTSTP);
    signal(SIGCHLD, childHandler);
    
    fg_pgid = getpid();

    if (argc > 1){ // script mode
        FILE *fptr = fopen(argv[1], "r");
        while (fgets(buffer, sizeof(buffer), fptr)){
            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(instruct, buffer);

            Instructions[0] = instruct;
            Instructions[1] = prevInstruct;

            if (strncmp(Instructions[0], "exit", 4) == 0){   
                exit = atoi(strncpy(temp, instruct + 4, 251));
                printf("bye lol\n");
                break;
            }

            if (strncmp(Instructions[0], "!!", 2) == 0){
                mode = checkCm(Instructions[1]); // get like !!
                continue;
            }
            else{
                mode = checkCm(Instructions[0]); // Check the commands and runs it
            }

            // Operations for dealing with cmd list.
            if (mode == 1){ // case where copy does not req.
                continue;
            }
            else if (mode == 0){ // Normal case
                strcpy(prevInstruct, instruct);
            }
            else if (mode == -1){
                printf("Command does not exist u dumb >:(\n");
                continue;
            }
        }
        fclose(fptr);
        return (uint8_t)exit;
    }

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, getpid()); //give term access to the main.

    while (1){ // Normal mode, user input thing
        
        
        fflush(stdout);
        printf("icsh $ "); 
        fgets(buffer, 255, stdin); 
        buffer[strcspn(buffer, "\n")] = 0;

        fflush(stdout);
        if (strcmp(buffer, "") == 0){
            continue;
        }

        strcpy(instruct, buffer);

        Instructions[0] = instruct;
        Instructions[1] = prevInstruct;

        if (strncmp(Instructions[0], "exit ", 5) == 0){
            exit = atoi(strncpy(temp, instruct + 4, 251));
            printf("bye lol\n");
            break;
        }

        else if (strncmp(Instructions[0], "jobs", 4) == 0){
            printProcess();
            continue;
        }

        // if (strncmp(Instructions[0], 'bg', 2) == 0){
        //     strtok(instruct, " ");
        //     char* processNum = strtok(NULL, " ");
        //     int pNum = atoi(processNum);
        //     pid_t pid = processMap[pNum-1][1];
        // }

        else if (strncmp(Instructions[0], "fg", 2) == 0){
            strtok(instruct, " ");
            char* processNum = strtok(NULL, " ");
            int pNum = atoi(processNum);
            pid_t pid = processMap[pNum - 1][1];

            printf("%d\n", pid);
            tcsetpgrp(STDIN_FILENO, pid); // bring the process up
            kill(-pid, SIGCONT); // continue
            
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpid());
            removePIDMap(pid); // remove it.
            fflush(stdin);
            continue;
        }

        else if (strncmp(Instructions[0], "!!", 2) == 0){
            mode = checkCm(Instructions[1]); // get like !!
            continue;
        }
        else{
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
