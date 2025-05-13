

#include <map>
#include <vector>
#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include "spawn_processes.h"
#include <fstream>



class job {
    public:
        static int size;
        int order;
        pid_t pid;
        string command;
        bool is_background;
        bool is_suspended;
 
    bool flip_state(){
        return !is_background; 
    }

    void cout_job(){
        cout << "[ ]" << pid << command << "\n";
    }

};

int job::size = 0;

