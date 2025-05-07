#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

#include "i_o_redirect.h"
#include "process_map.h"
#include "processes.h"

int detect_background(char* instruct){
    if (strchr(instruct, '&') != NULL){
        return 1;
    }
    return 0;
}

int spawn_processes(process_map* processes, char* instruct, pid_t child_pid){
    int reset_stdout = dup(STDOUT_FILENO);
    int reset_stdin = dup(STDOUT_FILENO);

    int is_background = detect_background(instruct);
    if (is_background == 1){
        instruct = strtok(instruct, "&");
    }

    int is_redirect = redirection(instruct);
    char* filename; 
    if (is_redirect != 0){
        filename = parse_redirect(instruct);
    }

    switch (is_redirect){
        case 1: //>
            instruct = strtok(instruct, ">");
            output(filename);
            break;
        case -1: //<
            instruct = strtok(instruct, ">");
            input(filename);
            break;
    }

    char* prog_arv[255];
    char* command = strtok(instruct, " ");

    char path[100] = "/bin/"; //mac
    // char path[100] = "/use/bin"; // linux
    prog_arv[0] = strcat(path, command);

    int index = 1;
    while (command != NULL){
        command = strtok(NULL, " ");
        prog_arv[index] = command;
        index++;
    }
    prog_arv[index] = NULL; // command thing.

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    child_pid = fork();
    add_pid_to_map(processes, child_pid, is_background);

    if (child_pid == 0){
        printf("hi\n");
        signal(SIGTTOU, SIG_IGN);
        signal(SIGINT, SIG_DFL); // specify default signal action.
        signal(SIGTSTP, SIG_DFL);

        setpgid(0, 0); // set group id to its own uniqe group setpgid(getpid(), getpid())
        
        int err = execvp(prog_arv[0], prog_arv); // will run the whole list
        fflush(stdout);
        if (err == -1){
            return -1;
        }
    }
    setpgid(child_pid, child_pid);
    int status;
    if (is_background == 0){
        tcsetpgrp(STDIN_FILENO, child_pid);
        waitpid(child_pid, &status, WUNTRACED);

        remove_pid_map(processes, child_pid);
        tcsetpgrp(STDIN_FILENO, getpid());
    }
    else if (is_background == 1){
        printf("%d is running.\n", getpid());
    }

    dup2(reset_stdout, STDOUT_FILENO);
    dup2(reset_stdin, STDIN_FILENO);

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
}
