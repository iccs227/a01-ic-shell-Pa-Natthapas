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
    addLast(Sentinel, token);
    while (token != NULL){
        token = strtok(NULL, " "); //Get the rest into the list.
        addLast(Sentinel, token);
        
    }
    // printLL(Sentinel);
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

void checkCm(Node* instruct){
    char* command = getItems(instruct, 0);
    if ( strcmp(command, "echo") == 0 ){
        echo(instruct);
    }
    else if ( strcmp(command, "!!") == 0 ){
        
    }
}

int main() {
    char buffer[MAX_CMD_BUFFER];
    Node* instruct;

    while (1) {
        instruct = LinkedList();
        printf("icsh $ ");
        fgets(buffer, 255, stdin);
        splitString(buffer, instruct); //Turn the string into a list
        checkCm(instruct); //Check the commands and runs it

        // printf("you said: %s\n", buffer);
    }
}
