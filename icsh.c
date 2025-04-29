/* ICCS227: Project 1: icsh
 * Name:
 * StudentID:
 */
// #include "LinkedList.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CMD_BUFFER 255


pid_t pid;

int outsideProcess(char* instruct){ //Pass in Commands that are already splitted
    char* prog_arv[255];
    char* command = strtok(instruct, " "); //First part of the instruct, command part.

    char str[100] = "/usr/bin/";
    prog_arv[0] = strcat(str, command);

    int index = 1;
    while (command != NULL){
        command = strtok(NULL, " ");
        prog_arv[index] = command;
        index++;
    }

    
    pid = fork();

    if (pid == 0){
        //child process
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        // tcsetpgrp(STDIN_FILENO, getpid()); //sets the child as foreground 
        int err = execvp(prog_arv[0],prog_arv); // will run the whole list 
        if (err == -1){
            return -1;
        }
        //this replace the memory iff it is successful 
    }

    int status;
    wait(&status);
    tcsetpgrp(STDIN_FILENO, getpid()); //set the main back to the forground

    int Stopped = 0;
    if (WIFSTOPPED(status)){
        Stopped = pid;
    }



    //3 cases 
    //1 is if program dne, 2 ping is found but error
    if (WIFEXITED(status)){ //if true then it means it exited normally
        int statusCode = WEXITSTATUS(status);
        if (statusCode == 0){
            return 0;
        }
        else{
            return -1;
        }
    }
    return 0;
    //main process
}


void echo(char* instruct){ //Printing stuff
        for (int i=5;i<strlen(instruct); i++){ 
            printf("%c", instruct[i]);
        }
        printf("\n");
    }


int checkCm(char* commands){ // 0 is current com, 1 is prev

    if ( strncmp( commands, "echo", 4 ) == 0 ){
        echo(commands);
        return 0;
    }

    else{
        int isExist = outsideProcess(commands);
        if (isExist == 0){ //does exist
            return 0;
        }
    }
    return -1; // command does not exist
}

int main(int argc, char* argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char instruct[MAX_CMD_BUFFER];
    char prevInstruct[MAX_CMD_BUFFER];
    char temp[MAX_CMD_BUFFER];

    
    char* Instructions[2] = {NULL, NULL};
    int mode;
    uint8_t exit = 0;


    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);


    if (argc > 1){ // script mode
        FILE *fptr = fopen(argv[1], "r");
        while (fgets(buffer, sizeof(buffer), fptr)){
            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(instruct, buffer);

            Instructions[0] = instruct;
            Instructions[1] = prevInstruct;

            if ( strncmp( Instructions[0], "exit", 4 ) == 0 ){
                exit = atoi(strncpy(temp, instruct+4, 251));
                printf("bye lol\n");
                break;
            }

            if ( strncmp(Instructions[0], "!!", 2) == 0 ){
                mode = checkCm(Instructions[1]); //get like !!
                continue;
            }
            else{
                mode = checkCm(Instructions[0]); //Check the commands and runs it
            }

    
            if (mode == 1){ // case where copy does not req.
                continue;
            }
            else if (mode == 0){ //Normal case
                strcpy(prevInstruct, instruct);
            }


            if (mode == 1){ // !! does not keep prev
                continue;
            }
            else if (mode == 0){ //Normal case, does not work for outside process.
                // printf("hi from mode==0\n");
                strcpy(prevInstruct, instruct);
            }
            else if (mode == -1){
                printf("Command does not exist u dumb >:(\n");
            }
        }    
        fclose(fptr);
        return (uint8_t)exit;  
    }


    while (1) {  //Normal mode, user input thing                
        fflush(stdin);
        printf("icsh $ ");

        fgets(buffer, 255, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        strcpy(instruct, buffer);
        
        Instructions[0] = instruct;
        Instructions[1] = prevInstruct;

        if ( strncmp( Instructions[0], "exit", 4 ) == 0 ){
            exit = atoi(strncpy(temp, instruct+4, 251));
            printf("bye lol\n");
            break;
        }
        
        if ( strncmp(Instructions[0], "!!", 2) == 0 ){
            mode = checkCm(Instructions[1]); //get like !!
            continue;
            }
        else{
            mode = checkCm(Instructions[0]); //Check the commands and runs it
        }


        if (mode == 1){ // case where cpy is not req.
            continue;
        }
        else if (mode == 0){ //Normal case
            strcpy(prevInstruct, instruct);
        }
        else if (mode == -1){
            printf("Command does not exist u dumb >:(\n");
        }
    }

    return (uint8_t)exit;
}
