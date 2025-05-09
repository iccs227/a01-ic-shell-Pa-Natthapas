#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include "icsh.h"


#include <iostream>
#include <stack>
#include <string>
#include <sstream>

using namespace std;

void output(string filename){
    int file_desc = open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(file_desc, STDOUT_FILENO);
    close(file_desc);
}

void input(string filename){
    int file_desc = open(filename.c_str(), O_RDONLY, 0777);
    dup2(file_desc, STDIN_FILENO);
    close(file_desc);
}

