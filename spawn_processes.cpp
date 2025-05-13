#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>


#include <map>
#include <stack>
#include <string>
#include <sstream>
#include "i_o_redirect.h"
#include <iostream>
#include <vector>
#include "icsh.h"
#include "jobs.h"
#include "signal.h"


using namespace std;

pid_t foreground_pid = -1;

void bring_foreground_from_background(string instruct){ // brings the pid up to the foreground. Only on running process, ground_state = 0
    int pid;
    int percent_index = instruct.find('%');//%
    int pid_order;
    if (percent_index == -1){
        cout << "index not specified.\n";
        return;
    } else {
        pid_order = stoi(instruct.substr(percent_index+1));
    }

    if (pid_order >= pid_vector.size()){
        cout << "out of range.\n";
        return;
    }

    if (ground_state[pid_order] == 0){
        ground_state[pid_order] = 1;
        pid = pid_vector[pid_order][0];
        string command = pid_to_command.at(pid);
        cout << command;
        ground_state[pid] = 0;
        for (int i = 0; i < pid_vector.size(); i++){ // remove dead_child.
            if (pid_vector[i][0] == pid){
                pid_vector[i][2] = 1;
            }
        }
        kill(pid, SIGCONT);
        return;
    }

    string command = pid_to_command.at(pid_vector[pid_order][0]).substr(0, pid_to_command.at(pid_vector[pid_order][0]).size()-1); // Extract the command.
    cout << "[" << pid_order << "] " << pid_vector[pid_order][0] << "\t\t " << command << "\n";
    tcsetpgrp(STDIN_FILENO, pid_vector[pid_order][0]);
    wait(NULL);
    tcsetpgrp(STDIN_FILENO, getpid()); // set process back.
}

void bring_stop_to_background(string instruct){ // 
    int pid;
    int percent_index = instruct.find('%');//%
    int pid_order;
    if (percent_index == -1){
        cout << "index not specified.\n";
        return;
    } else {
        pid_order = stoi(instruct.substr(percent_index+1));
    }

    if (pid_order >= pid_vector.size()){
        cout << "out of range.\n";
        return;
    }
     
    pid = pid_vector[pid_order][0];
    string command = pid_to_command.at(pid);
    cout << command << " &\n";
    ground_state[pid] = 0;
    for (int i = 0; i < pid_vector.size(); i++){ // remove dead_child.
        if (pid_vector[i][0] == pid){
            pid_vector[i][2] = 1;
        }
    }
    kill(pid, SIGCONT);
}

void cout_vec(vector<int> vec){
    cout << "{ ";
    for (int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }
    cout << "}";
}

int is_redirect(string instruction){
    if (instruction.find('>') != -1){
        instruction = instruction.substr();
        return 1;
    } 
    else if (instruction.find('<') != -1){
        return -1;
    }
    return 0;
}


int spawn_processes(string instruction){
    string cpy = instruction;
    int reset_stdout = dup(STDOUT_FILENO);
    int reset_stdin = dup(STDIN_FILENO);

    signal(SIGCHLD, handle_child);

    int is_background = 0; // 0 is not bg, 1 is the bg
    bool is_bg = false;
    if (instruction.find('&') != -1){ // check if init as bg process.
        instruction.erase(instruction.size() - 2);
        is_background = 1; // 1 is bg
        is_bg = true;
    }

    int redirect_flag = is_redirect(instruction);
    int flag_loc;
    string filename;

    if (redirect_flag != 0){
        switch (redirect_flag){
            case 1: // >
                flag_loc = instruction.find('>');
                filename = instruction.substr(flag_loc+2);
                instruction = instruction.substr(0, flag_loc);
                output(filename);
                break;
            case -1: // <
                flag_loc = instruction.find('<');
                filename = instruction.substr(flag_loc+2);
                instruction = instruction.substr(0, flag_loc);
                input(filename);
                break;
        }
    }

    char* prog_arv[255];
    char buffer[256]; // get extra 1 for null term
    strncpy(buffer, instruction.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    char* command = strtok(buffer, " ");

    char path[100] = "/bin/"; // mac
    // char path[100] = "/usr/bin/" // linux
    prog_arv[0] = strcat(path, command);

    int index = 1;
    while (command != NULL){
        command = strtok(NULL, " ");
        prog_arv[index] = command;
        index++;
    }
    prog_arv[index] = NULL;

    int size = pid_vector.size();
    
    int child_pid = fork();
    if (!is_bg){
        foreground_pid = child_pid;
    }
    

    if (child_pid == 0){
        setpgid(0, 0);
        
        signal(SIGINT, SIG_DFL); // specify default signal action.
        signal(SIGTSTP, SIG_DFL); 

        int fail_flag = execvp(prog_arv[0], prog_arv); 

    
        if (fail_flag == -1){ //remove command if it is not found
            for (int i = 0; i < job_vec.size(); i++){ // test
                if (job_vec[i].pid == child_pid){
                    job_vec.erase(job_vec.begin() + i);
                    break;
                }
            }
        }
    }
    setpgid(child_pid, child_pid);

    job a_job;
    a_job.pid = child_pid; // test
    a_job.size++; // test
    a_job.is_background = is_bg; // test
    a_job.is_suspended = false;
    job_vec.push_back(a_job);

    // if (!is_bg){
    //     tcsetpgrp(STDIN_FILENO, child_pid);
    //     int status;
    //     waitpid(-1, &status, WUNTRACED);
    //     foreground_pid = -1;

    //     if (WIFSTOPPED(status)){
    //         for (int i = 0; i < job_vec.size(); i++){
    //             if (job_vec[i].pid == child_pid){
    //                 job_vec[i].is_suspended = true;
    //             }
    //         }
    //         tcsetpgrp(STDIN_FILENO, shell_pid);
    //     }

    // }
    
    dup2(reset_stdout, STDOUT_FILENO); // Reset stdin/out
    dup2(reset_stdin, STDIN_FILENO);
    tcsetpgrp(STDIN_FILENO, shell_pid);
    
    return 0;
}   

// void foreground(){
//     tcsetpgrp(STDIN_FILENO);
// }

// void background(){

// }