
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>


#include <map>
#include <vector>
#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include "spawn_processes.h"
#include <fstream>
#include "icsh.h"
#include "jobs.h"

using namespace std;

#define MAX_CMD_BUFFER 256

//* Send -> Change the state to suspend -> kill(SIGSTOP) -> The process is still there. */
void child_tstp_handler(int sig){
    int status;
    pid_t sent_pid = waitpid(-1, &status, WUNTRACED);
    cout << sent_pid;
    for (int i ; i < job_vec.size(); i++){
        if (job_vec[i].pid == sent_pid){
            job_vec[i].is_suspended = false;
        }
    }
}

void tstp_handler(int sig){
    cout << "process suspended.\n";
}

void handle_child(int sig){
    int status;
    char msg[MAX_CMD_BUFFER];
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){ // WNOHANG tells waitpid not to block if no child has exited, WUNTRACED return if child has been paused.
        for (int i = 0; i < job_vec.size(); i++){
                if (job_vec[i].pid == pid){
                    if (job_vec[i].is_background == false){
                        job_vec.erase(job_vec.begin() + i);
                        job::size--;
                        tcsetpgrp(STDIN_FILENO, shell_pid);
                        break;
                    }
                //printing shit
                int len = snprintf(msg, sizeof(msg), "\nChild [%d]  %d exited.\t\t %s\n\033[34mi\033[32mc\033[36ms\033[35mh \033[33m$ \033[37m", job_vec[i].order, pid, job_vec[i].command.c_str());  
                write(STDOUT_FILENO, msg, len);
                job_vec.erase(job_vec.begin() + i);
                job::size--;
                break;
            }
        }
    }
    tcsetpgrp(STDIN_FILENO, shell_pid);
}


