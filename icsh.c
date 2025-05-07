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

#include "process_map.h"
#include "i_o_redirect.h"
#include "processes.h"


#define MAX_CMD_BUFFER 255

// pid_t suspended_pid = -1;
pid_t child_pid;
pid_t mainBranch;
pid_t fg_pgid;

process_map* processes = NULL;

void echo(char *instruct){ // Printing stuff
    for (int i = 5; i < strlen(instruct); i++){
        printf("%c", instruct[i]);
    }
    printf("\n");
}

int checkCm(process_map* processes, char *commands){ // 0 is current com, 1 is prev
    if (strncmp(commands, "echo ", 5) == 0){
        echo(commands);
        return 0;
    }
    else{
        int isExist = spawn_processes(processes, commands, child_pid);
        if (isExist == 0){ // does exist
            return 0;
        }
    }
    return -1; // command does not exist
}


void childHandler(int sig){   // Still fucks up my format part. 

    fflush(stdout);

    int status;
    pid_t deadChild;
    
    char msg[MAX_CMD_BUFFER];
    
    while ((deadChild = waitpid(-1, &status, WNOHANG)) > 0){ //
        fflush(stdout);
        int pidOrder = remove_pid_map(processes, deadChild);
        int len = snprintf(msg, sizeof(msg), "\nChild [%d] %d exited.\nicsh $ ",pidOrder, deadChild);
        fflush(stdout);
        fflush(stdin);
        write(STDOUT_FILENO, msg, len);
        fflush(stdout);
    }
    fflush(stdout);
    
}

void handleTSTP(int sig){
    add_pid_to_map(processes, child_pid, 0);
    kill(-child_pid, SIGSTOP);
    tcsetpgrp(STDIN_FILENO, getpid());
}


// END of Handler
//####################  ################################


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
    processes = new_process_map(); // yay

    if (argc > 1){ // script mode
        FILE *fptr = fopen(argv[1], "r");
        while (fgets(buffer, sizeof(buffer), fptr)){
            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(instruct, buffer);

            Instructions[0] = instruct;
            Instructions[1] = prevInstruct;

            if (strncmp(Instructions[0], "exit ", 5) == 0){
                exit = atoi(strncpy(temp, instruct + 4, 251));
                printf("bye lol\n");
                break;
            }
    
            else if (strncmp(Instructions[0], "jobs", 4) == 0){
                print_process(processes);
                continue;
            }
    
            else if (strncmp(Instructions[0], "fg", 2) == 0){
                strtok(instruct, " ");
                char* processNum = strtok(NULL, " "); // get the number
                int pNum = atoi(processNum); // order
                pid_t pid = get_id_by_order(processes, pNum);
    
                tcsetpgrp(STDIN_FILENO, pid); // bring the process up
                kill(-pid, SIGCONT); // continue
                
                int status;
                waitpid(pid, &status, WUNTRACED);
                tcsetpgrp(STDIN_FILENO, getpid());
                remove_pid_map(processes, pid); // remove it.
                fflush(stdin);
                continue;
            }
    
            else if (strncmp(Instructions[0], "!!", 2) == 0){
                mode = checkCm(processes, Instructions[1]); // get like !!
                continue;
            }
            else{
                mode = checkCm(processes, Instructions[0]); // Check the commands and runs it
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
            print_process(processes);
            continue;
        }

        else if (strncmp(Instructions[0], "bg", 2) == 0){
            strtok(instruct, " ");
            char* processNum = strtok(NULL, " "); // get the number
            int pNum = atoi(processNum); // order
            pid_t pid = get_id_by_order(processes, pNum);
            kill(-pid, SIGCONT);

            printf("%s & \n", Instructions[0]);

            int status;
            waitpid(pid, &status, WUNTRACED);
            remove_pid_map(processes, pid); // remove it.
            fflush(stdout);
            continue;

        }

        else if (strncmp(Instructions[0], "fg", 2) == 0){
            strtok(instruct, " ");
            char* processNum = strtok(NULL, " "); // get the number
            int pNum = atoi(processNum); // order
            pid_t pid = get_id_by_order(processes, pNum);

            tcsetpgrp(STDIN_FILENO, pid); // bring the process up
            kill(-pid, SIGCONT); // continue
            
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpid());
            remove_pid_map(processes, pid); // remove it.
            fflush(stdout);
            continue;
        }

        else if (strncmp(Instructions[0], "!!", 2) == 0){
            mode = checkCm(processes, Instructions[1]); // get like !!
            continue;
        }
        else{
            mode = checkCm(processes, Instructions[0]); // Check the commands and runs it
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

    free(processes);
    return (uint8_t)exit;
}
