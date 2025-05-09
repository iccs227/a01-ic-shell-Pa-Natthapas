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

using namespace std;

void bring_foreground_from_background(string instruct){ // brings the pid up to the foreground. Only on running process, ground_state = 0
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
        cout << "Cannot bring up a suspended process.\n";
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

    if (ground_state[pid_order] == 1){
        cout << "Cannot bring up a running process.\n";
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

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    int is_background = 0; // 0 is not bg, 1 is the bg
    if (instruction.find('&') != -1){ // check if init as bg process.
        instruction.erase(instruction.size() - 2);
        is_background = 1; // 1 is bg
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
    vector<int> vec; // add commands, etc etc.
    vec.insert(vec.begin(), child_pid);
    vec.insert(vec.begin() + 1, size);
    vec.insert(vec.begin() + 2, is_background);
    pid_vector.push_back(vec); 
    pid_to_command.insert(make_pair(child_pid, cpy));
    ground_state.insert(make_pair(child_pid, 0));
    
    if (child_pid == 0){
        signal(SIGTTOU, SIG_IGN); 
        signal(SIGINT, SIG_DFL); // specify default signal action.
        signal(SIGTSTP, SIG_DFL); //Send to the child process, so var change only in background, not the actual global one
        setpgid(0, 0);

        int fail_flag = execvp(prog_arv[0], prog_arv);

        if (fail_flag == -1){ //remove command if it is not found
            for (int i = 0; i < pid_vector.size(); i++){
                if (pid_vector.at(i)[0] == child_pid){
                    pid_vector.erase(pid_vector.begin() + i);
                    pid_to_command.erase(child_pid);
                    break;
                }  
            } 
            return -1;
        }
    }
    setpgid(child_pid, child_pid);
    int status;

    if (is_background == 0 && ground_state[child_pid] == 0){
        tcsetpgrp(STDIN_FILENO, child_pid);  
        waitpid(-1, &status, WUNTRACED);     // run normally.
        if (!WIFSTOPPED(status)){
            for (int i = 0; i < pid_vector.size(); i++){
                if (pid_vector.at(i)[0] == child_pid){
                    pid_vector.erase(pid_vector.begin() + i);
                    pid_to_command.erase(child_pid);
                    break;
                }  
            }
        }
        else {
            ground_state[child_pid] = 1;
        }
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
    else if (is_background == 1){
        printf("[%d] %d is running.\n",size, child_pid);
    }


    dup2(reset_stdout, STDOUT_FILENO);
    dup2(reset_stdin, STDIN_FILENO);
    tcsetpgrp(STDIN_FILENO, getpid());
    
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

