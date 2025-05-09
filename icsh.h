#ifndef ICSH_H
#define ICSH_H

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

extern pid_t child_pid;
extern vector<vector<int> > pid_vector;
extern stack<string> command_stack;
extern map<int, string> pid_to_command;
extern map<int, int> ground_state;

void check_command(string instruct);
int main(int argc, char *argv[]);


#endif