#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>


int redirection(char* instruct){ // 1 is for >, -1 is for <.
    if (strchr(instruct, '>') != NULL){
        return 1;
    } 
    else if (strchr(instruct, '<') != NULL){
        return -1;
    }
    return 0;
}

void output(char filename[]){
    printf("%s\n", filename);
    int file_desc = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(file_desc, STDOUT_FILENO);
    close(file_desc);
}

void input(char filename[]){
    printf("%s\n", filename);
    int file_desc = open(filename, O_RDONLY, 0777);
    dup2(file_desc, STDIN_FILENO);
    close(file_desc);
}

char* parse_redirect(char* instruct){ // returns the filename
    char file[255];
    char *arr = file;
    int index_out = 0;
    while (instruct[index_out] != '>'){
        index_out++;
    }
    for (int i = index_out; i < strlen(instruct); i++){
        file[i-index_out] = instruct[i];
    }
    strtok(file, " ");
    arr = strtok(NULL, " ");

    char* ret = malloc(strlen(arr) + 1);
    strcpy(ret, arr);

    return ret;
}