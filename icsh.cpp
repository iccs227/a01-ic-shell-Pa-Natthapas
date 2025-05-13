
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
#include "jobs.h"
#include "signal.h"

#define MAX_CMD_BUFFER 256

using namespace std;

pid_t child_pid;
pid_t shell_pid;

vector<vector<int> > pid_vector; // { { pid, order, isBackground }, ... } 
vector<job> job_vec; // {job, job, job, ...}

stack<string> command_stack; 
map<int, string> pid_to_command;
map<int, int> ground_state; // // State given by 0 = running, 1 = suspended


void welcome_message(){
    string colors[] = {
        "\033[31m", //red
        "\033[33m", //yellow
        "\033[32m", //green
        "\033[36m", //cyan
        "\033[34m", //blue
        "\033[35m", //magenta
        "\033[0m"
    };
    
    
    cout <<  "\033[31m   ⢠⢀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀  \n";
    cout <<  "\033[33m⢻⣼⢧⣄⠈⠀⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀  \n";
    cout <<  "\033[32m⢸⣠⣟⣿⣿⣦⣄⠀⠀⠀⠈⠈⠈⠀⠀⠀⠐⠐⠀⠂⠂⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀  \n";
    cout <<  "\033[36m⠀⢻⣿⣶⣻⣿⣿⣿⣦⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⠠⠂⠀⠀⠀⠀⠀⠀⠀⣠⣴⠟  \n";
    cout <<  "\033[34m⠠⠀⢻⣿⣿⣧⣙⠻⠿⣿⣿⣶⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⠀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⣴⠿⠋⠀⠀  \n";
    cout <<  "\033[35m⠀⠀⠀⠻⣿⣿⣿⣷⣷⣬⡭⣙⢻⠿⣿⣷⣦⣤⣤⣤⣤⣴⣤⣤⣶⣶⣿⣿⣷⣿⣷⣿⣾⣿⣿⣿⡿⣿⣿⣷⣿⣾⣷⡿⢿⡿⣿⣶⣶⣤⣤⣤⣶⣶⡿⠿⠟⡛⢧⠐⠠⣄   \n";
    cout <<  "\033[31m   ⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣻⣿⣿⣿⣿⢿⣿⣿⣿⠿⣿⣿⣿⣿⠿⣿⣿⣿⣿⣿⣿⣟⣿⣿⡟⠛⢻⣿⣧⣛⠸⣃⡿⣿⣿⡟⠿⠿⣸⣧⠘⡄⠃⠄⢃⣤⣿⠃  \n";
    cout <<  "\033[33m   ⠀⠀⣿⣿⣿⣿⣿⣿⣿⣯⣽⣿⣟⠛⡛⣿⣿⣿⡿⠫⢅⠊⡔⠛⡿⣿⣾⣽⣿⣿⣿⣿⣿⣿⣿⡟⡀⠀⠀⠘⢻⣿⣷⠈⠑⠛⠻⣿⣏⣲⡁⢎⡳⠈⠅⢺⣤⣿⠃⠀  \n";
    cout <<  "\033[32m   ⠀⠀⠀⠙⣿⣿⣿⣿⣿⣿⡿⢟⡻⢛⢂⠱⢿⣿⣏⠲⣁⠎⡘⢄⠣⡐⢌⡹⢿⣿⣿⣿⡟⢻⢋⠙⡉⠄⠠⠀⠌⢈⣿⣿⡇⠀⠀⠳⠿⠻⢿⣿⡀⠆⡉⠠⣽⣾⠃⠀⠀ \n";
    cout <<  "\033[36m   ⠀⠀⠀⠀⠈⣿⣿⣿⣿⢳⡘⢦⡐⡁⠂⠌⠚⡹⢁⠓⡌⢆⡘⢤⠣⢜⢢⣽⣿⢫⠝⢦⡙⠢⠌⡐⠰⡈⠄⠡⠀⠂⠼⠟⠁⠀⠀⠀⠀⡀⠈⠹⣿⣦⣔⠙⣿⡇⠀⠀  \n";
    cout <<  "\033[34m       ⢠⣿⣿⣿⣿⣷⣿⣿⣿⣷⣷⣤⣂⠑⢦⡈⠔⣂⠹⣶⣙⣮⣿⢻⡌⢧⡚⣥⢎⠡⢊⡔⣿⠄⡁⠂⠡⠀⣀⣦⣼⣶⣾⣿⣿⣾⣷⣦⣜⣇⣟⣈⢿⡇⠀⠀⠀ \n";
    cout <<  "\033[35m   ⠀⠀⠀⣰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⡀⠣⢄⢋⢿⣿⣿⢏⣳⢚⡧⢳⣼⢃⠎⣡⢺⣿⠀⠀⠀⢤⣃⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⣻⣾⡇⠀⠀⠀  \n";
    cout <<  "\033[31m   ⠀⠀⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣆⠂⡜⢺⣿⣿⣏⢾⡹⣜⣻⡟⣌⠚⣤⣿⡇⠀⠀⠀⣱⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠛⢻⣾⣿⡇⠀⠀⠀  \n";
    cout <<  "\033[33m   ⠀⣸⣿⣿⡿⣟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⢌⡹⣿⣿⡞⣧⢻⣼⣿⡗⠤⡃⠔⡋⡄⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⢀⠈⣿⡇⠀⠀⠀  \n";
    cout <<  "\033[32m   ⠀⣿⣿⣿⢣⢟⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢦⢹⣿⣿⡜⡧⣏⣾⣿⡝⡼⢋⢆⡑⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡏⠀⠀⣼⣿⡀⠀⠀  \n";
    cout <<  "\033[36m   ⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣆⣦⣸⣼⣿⣿⣿⣿⣿⣿⣿⡽⣏⣿⡱⢍⠳⣎⢿⡹⢮⣳⡟⣮⢳⣿⣿⣿⣿⣦⣴⣶⣶⣿⣿⣿⣿⡿⣏⠳⡜⢣⠞⣼⣿⠀⠀  \n";
    cout <<  "\033[34m   ⠀⣿⣿⣿⣷⠾⡙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣬⣿⣿⡿⣽⣳⡽⣾⡿⣿⣷⡩⢖⣬⢣⡔⣨⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠋⠀⣀⣶⣿⣿⣇⠀⠀  \n";
    cout <<  "\033[35m   ⠀⣿⣿⣿⣿⣿⣷⣦⣍⣙⠛⡛⢛⠛⢛⠛⣟⣿⣿⣿⣿⣿⣿⡿⣽⣳⣿⡻⣇⠻⣵⢻⣿⣏⢶⣫⢽⣹⣿⣿⣮⠣⠍⠛⠛⠛⠛⢉⡉⣤⣰⣶⣿⡿⢿⣿⢿⣿⠀⠀  \n";
    cout <<  "\033[31m   ⠀⢸⣿⣿⣿⣿⣿⣷⡿⣾⣟⣿⡿⣿⢿⣭⣿⣿⣿⣿⣿⣿⣿⣿⣿⣻⢵⡚⢷⡈⠳⣬⢳⡹⢧⡳⣏⢷⡻⢿⣿⣿⣿⣿⣽⡿⢿⣻⣿⢶⣣⢓⡜⡛⠼⡳⠿⣛⣿⠄⠀ \n";
    cout <<  "\033[33m   ⠀⠀⣿⣿⣿⣿⣿⣿⣿⣟⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣝⢶⣩⠒⡭⣑⢮⡱⣙⢦⣽⣾⣿⠿⣛⠛⣿⣿⣿⣧⡛⢧⣛⢾⡣⢇⡫⣔⢫⠖⡑⣢⣽⣿⠀⠀ \n";
    cout <<  "\033[32m   ⠀⠀⠹⣿⣿⣿⣿⣿⣿⣿⣿⣟⣿⣿⣻⢾⣯⢿⣹⣿⣿⣿⣿⣿⣿⡽⣞⣧⠿⣷⢿⣿⣿⣿⣿⣿⣿⣏⣾⣶⡟⢋⠉⠻⣿⣿⣬⢣⡛⣜⡡⢖⡩⣻⣮⣴⣿⣟⣿⠀⠀ \n";
    cout <<  "\033[36m   ⠀⠀⠀⠘⣿⣿⣿⣿⣿⣿⣿⡿⣟⣾⣟⡿⣜⢧⣳⠾⡽⣿⣿⣿⣿⣿⣿⣞⣿⣭⣿⢾⣽⣳⣿⣾⣿⣿⣿⠟⠀⠄⠈⠐⡀⣛⠿⡷⣙⠚⡅⠋⡔⠹⣿⢿⣿⣯⣿⠀⠀ \n";
    cout <<  "\033[34m   ⠀⠀⠀⠀⠘⣿⣿⣿⣿⣽⣷⡟⣿⣿⣿⣽⢻⡞⣧⡟⣵⢣⠛⣿⣿⣿⣿⣿⢳⣯⡟⣿⡞⣿⣿⣿⡟⠋⠁⠀⣤⠈⠀⡔⣴⣯⣶⣵⠈⠂⠀⠁⠀⢡⠐⣬⣿⣿⣿⠀⠀ \n";
    cout <<  "\033[35m   ⠀⠀⠀⠀⠀⠈⢻⣿⣿⣽⣻⣽⣿⢿⡿⣿⣿⣿⡿⢾⡵⢎⡳⢤⡩⢿⣿⣿⣿⣾⣿⣷⣯⣿⡿⠙⠀⠀⢀⠀⠀⠈⠀⠁⢉⠉⠉⠁⠃⠀⠀⢀⠢⢄⠂⡔⣿⣿⣿⠁⠀ \n";
    cout <<  "\033[31m    ⠀⠀⠀⠀⠀⠀⠘⣿⣿⣿⣷⣿⣾⣧⣿⣵⣯⣾⣽⣳⡞⣯⢞⡵⢎⡻⢽⣿⣿⣷⣿⣿⣿⡟⣁⠂⠠⠁⠀⠀⠀⠄⢀⠶⢠⠀⠄⡠⢀⠀⠀⠀⠂⠈⡐⢈⣿⣿⣿⠀ \n";
    cout <<  "\033[33m           ⠀⢿⣿⣿⣿⣾⣯⢿⡽⣯⡟⣿⣻⢯⡝⣧⢻⡜⣣⡙⢆⡞⣿⣿⣿⣿⢏⠴⣠⡀⠄⠂⠁⡀⠁⠀⠀⠀⠀⠀⠀⠀⠀⠈⡀⢂⠱⣀⠐⢢⣿⣿⡏ \n";
    cout <<  "\033[32m        ⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣯⣟⣷⣻⢷⣻⡾⣽⣞⣷⡹⢲⡝⡾⣜⣳⣿⣿⣿⣎⠶⡱⡉⢌⡑⢂⡐⠈⠃⠐⠠⠐⠀⠂⠌⠀⢄⡰⢉⠷⣎⡉⢶⣾⣿⠃ \n";
    cout <<  "\033[36m      ⠀⠀⠀⠀⠀⠀⣼⣿⡿⣿⢻⡿⣿⣿⣾⣯⣿⣳⣟⣧⢟⡾⣽⣷⣯⣳⣯⣿⣿⣿⣿⣾⣽⣣⠽⡆⠜⡠⠒⡤⠘⠤⣁⠂⠆⣀⠢⣉⠶⣌⢃⠒⡀⠙⣼⣿⠁⠀ \n";
    cout <<  "\033[34m    ⠀⠀⠀⠀⠀⠀⢠⣾⣿⣿⡹⣞⢧⡟⣯⢛⡿⢿⣿⣿⣾⣿⣾⣷⣯⣷⣯⣷⣿⡿⣟⠿⡿⢿⣿⣿⣷⣯⣼⣰⣥⣆⣍⠰⠠⢌⣰⣀⠒⡄⠓⠠⠈⢂⠄⢤⣾⡏⠀⠀ \n";
    cout <<  "\033[35m       ⠀⠀⣠⣿⣿⣿⣳⡝⣮⢫⡝⡲⢏⢺⢡⠫⣝⢻⣿⣿⢿⢿⡻⣟⢯⡳⣝⢮⡛⣜⢣⢎⡝⡹⢛⡛⣛⠛⢋⣥⣶⣷⡾⠛⠁⠂⠐⠈⠀⠀⠌⡘⣦⡿⠃⠀⠀ \n";
    cout <<  "\033[31m   ⠀⠀⠀⣀⣤⣿⣿⣿⣿⣳⠿⣜⣧⣾⣧⣭⣶⢃⠧⡘⢦⢩⢏⣻⣎⢷⣫⢷⡹⣎⣶⣹⣬⡓⢎⠴⣡⢃⠖⡠⢉⣂⢌⡉⢀⠀⣀⠀⠀⠀⢀⡀⠀⣌⣼⡟⠀⠀⠀⠀⠀ \n";    
}

void cout_vec_test(vector<int> vec){
    cout << "{ ";
    for (int i = 0; i < vec.size(); i++){
        cout << vec[i] << " ";
    }
    cout << "}";
}

void cout_vec_string(vector<string> vec){
    cout << "{ ";
    for (int i = 0; i < vec.size(); i++){
        cout << vec[i] << ", ";
    }
    cout << "}";
}

void cout_format(){
    // for (int i = 0;i < pid_vector.size(); i++){ // vector is kept as {pid, order}, map is kept as {pid, command}
    //     char msg[MAX_CMD_BUFFER];
    //     int len;
    //     if (ground_state[pid_vector[i][0]] == 0){ // process is running
    //         len = snprintf(msg, sizeof(msg), "[%d] %d Running.\t\t %s\n", pid_vector[i][1], pid_vector[i][0], pid_to_command.at(pid_vector[i][0]).c_str());
    //     }
    //     else { // is paused
    //         len = snprintf(msg, sizeof(msg), "[%d] %d Suspended.\t\t %s\n", pid_vector[i][1], pid_vector[i][0], pid_to_command.at(pid_vector[i][0]).c_str());
    //     }
    //     write(STDOUT_FILENO, msg, len);
    // }
    for (int i = 0; i < job_vec.size(); i++){
        job_vec[i].cout_job();
    }
}


void check_command(string instruct){
    if ((command_stack.top()).compare(0, 4, "echo", 0, 4) == 0){
        cout << command_stack.top().substr(5) << "\n"; 
    }
    else if ((command_stack.top()).compare(0, 2, "!!", 0, 2) == 0){
        command_stack.pop();
        if (command_stack.empty()){
            cout << "no previous commnad.\n";
            return;
        }
        check_command(command_stack.top());
    }
    else if ((command_stack.top()).compare(0, 4, "fg %", 0, 4) == 0){ // bring *Running process from background up
        bring_foreground_from_background(command_stack.top());
    }
    else if ((command_stack.top()).compare(0, 4, "bg %", 0, 4) == 0){ // bring *Suspended process from background up
        bring_stop_to_background(command_stack.top());
    }
    else if ((command_stack.top()).compare(0, 4, "jobs", 0, 4) == 0){
        cout_format();
    }
    else {
        int run_command = spawn_processes(instruct);
        if (run_command == -1){
            cout << "command not found.\n";
        }
    }
}

int main(int argc, char *argv[]){
    welcome_message();
    string buffer;
    string instruction;   
    uint8_t exit_code = 0;
    shell_pid = getpid();
    setpgid(0,0);

    signal(SIGINT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);

    if (argc > 1){ //script mode
        ifstream File(argv[1]);
        while (getline(File, buffer)){
            if (buffer.compare(0, 4, "exit", 0, 4) == 0){  
                if (buffer.length() == 4){
                    cout << "bye bye\n";
                    return 0;
                }
                exit_code = (uint8_t)stoi(buffer.substr(5));
                cout << "bye bye\n";
                return (uint8_t)exit_code;
            }
    
            if (buffer.empty()){
                continue;
            }
    
            command_stack.push(buffer);
            check_command(command_stack.top());
        }
    }

    while (1){
        fflush(stdout);
        cout << "\033[34mi\033[32mc\033[36ms\033[35mh \033[33m$ \033[37m";
        getline(cin, buffer);

        if (buffer.compare(0, 4, "exit", 0, 4) == 0){  
            if (buffer.length() == 4){
                cout << "bye bye\n";
                return 0;
            }
            exit_code = (uint8_t)stoi(buffer.substr(5));
            cout << "bye bye\n";
            return (uint8_t)exit_code;
        }

        if (buffer.empty()){
            continue;
        }

        command_stack.push(buffer);
        check_command(command_stack.top());
    }
    return (uint8_t)exit_code;
}
