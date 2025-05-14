#ifndef JOBS_H
#define JOBS_H

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
   
    void cout_job(){
        if (is_suspended == true){
            cout << "[" << order << "] " << pid << "\t\t" << "Suspended " << command << "\n";
            return;
        }
        cout << "[" << order << "] " << pid << "\t\t" << "Running " << command << "\n";

    }

};

// extern int job::size = 0;

#endif