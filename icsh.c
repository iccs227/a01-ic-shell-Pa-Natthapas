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

#define MAX_CMD_BUFFER 255

int outsideProcess(char* instruct){ //Pass in Commands that are already splitted
    char* prog_arv[255];
    char* command = strtok(instruct, " "); //First part of the instruct, command part.
    int retVal;
   
    char str[100] = "/usr/bin/";
    prog_arv[0] = strcat(str, command);

    int index = 1;
    while (command != NULL){
        command = strtok(NULL, " ");
        prog_arv[index] = command;
        index++;
    }

    
    int pid = fork();

    if (pid == 0){
        //child process
        execvp(prog_arv[0],prog_arv); // will run the whole list 
    }

    int check = waitpid(pid, NULL, 0);
    if (check == -1){ //File not found 
        return -1;
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


int checkCm(char* commands[], int oldNew){ // 0 is current com, 1 is prev

    if ( strncmp( commands[oldNew], "echo", 4 ) == 0 ){
        echo(commands[oldNew]);
        return 0;
    }

    else if ( strncmp( commands[oldNew], "exit", 4 ) == 0 ){
        return 2;
    }

    else if ( strncmp(commands[0], "!!", 2) == 0 ){
        if (commands[1] == NULL){
            return 1;
        }
        checkCm(commands, 1);
        return 1;
    }

    else{
        int isExist = outsideProcess(commands[oldNew]);
        if (isExist == 0){
            return 0;
        }
        printf("Command does not exist u baka. >:(\n");
        return 1;
    }
}


uint8_t main(int argc, char* argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char instruct[MAX_CMD_BUFFER];
    char prevInstruct[MAX_CMD_BUFFER];
    char temp[MAX_CMD_BUFFER];

    char* Instructions[2] = {NULL, NULL};

    uint8_t exit = 0;

    if (argc > 1){ // script mode
        FILE *fptr = fopen(argv[1], "r");
        while (fgets(buffer, sizeof(buffer), fptr)){
            buffer[strcspn(buffer, "\n")] = 0;

            strcpy(instruct, buffer);

            Instructions[0] = instruct;
            Instructions[1] = prevInstruct;

            int mode = checkCm(Instructions, 0); //Check the commands and runs it
            
            if (mode == 1){ // !!
                continue;
            }
            else if (mode == 0){ //Normal case
                strcpy(prevInstruct, instruct);
            }
            else if (mode == 2){ //Exit
                exit = atoi(strncpy(temp, instruct+4, 251));
                printf("bye lol\n");
                break;
            }
        }    
        fclose(fptr);
        return (uint8_t)exit;  
    }


    while (1) {  //Normal mode, user input thing                

        printf("icsh $ ");

        fgets(buffer, 255, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        strcpy(instruct, buffer);
        
        Instructions[0] = instruct;
        Instructions[1] = prevInstruct;
        
        int mode = checkCm(Instructions, 0); //Check the commands and runs it

        if (mode == 1){ // !!
            continue;
        }
        else if (mode == 0){ //Normal case
            strcpy(prevInstruct, instruct);
        }
        else if (mode == 2){ //Exit
            exit = atoi(strncpy(temp, instruct+4, 251));
            printf("bye lol");
            break;
        }
    }
    return (uint8_t)exit;
}
