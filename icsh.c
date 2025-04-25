/* ICCS227: Project 1: icsh
 * Name:
 * StudentID:
 */
#include "LinkedList.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_CMD_BUFFER 255

void splitString(char* buffer, Node* Sentinel){
    char* token = strtok(buffer, " "); //Get the first part.
    while (token != NULL){
        addLast(Sentinel, token);
        token = strtok(NULL, " "); //Get the rest into the list.
    }
}


void echo(Node* instruct){
    for (int i=1;i<getSize(instruct); i++){
        if (getItems(instruct, i) == NULL){
            break;
        }
        printf("%s ", getItems(instruct, i));
    }
    printf("\n");
}


int checkCm(Node* instruct[], int oldNew){ //1 for current 0 for prev
    Node* current = instruct[oldNew];
    char* command = getItems(current, 0);
   
    if ( strcmp(command, "echo") == 0 ){
        echo(instruct[oldNew]);
    }
    else if ( strcmp(command, "!!") == 0 ){
        if (instruct[1] == NULL){
            return 1;
        }
        
        for (int i=0;i<getSize(instruct[1]); i++){
        if (getItems(instruct[1], i) == NULL){
            break;
        }
        printf("%s ", getItems(instruct[1], i));
    }
        printf("\n");
        checkCm(instruct, 1);
        return 1;
    }   

    else{
        printf("Bad Command! >:(\n");
        return 1; // Mark for like skipping the copy thing
    }

    return 0; // The Function is not skipped, meaning it will be copied.
}


int main() {
    char buffer[MAX_CMD_BUFFER];
    Node* instruct = NULL;
    Node* prevInstruct = NULL;
    Node* Instructions[2];

    while (1) {
        instruct = LinkedList();
        printf("icsh $ ");

        fgets(buffer, 255, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        splitString(buffer, instruct); //Turn the string into a list
        
        Instructions[0] = instruct;
        Instructions[1] = prevInstruct;

        int mode = checkCm(Instructions, 0); //Check the commands and runs it

        if (mode == 1){
            continue;
        }
        else if (mode == 0){
            prevInstruct = copy(instruct);
        }
    }
}
