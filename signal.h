
#ifndef SIGNAL_H
#define SIGNAL_H

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

void child_tstp_handler(int sig);
void handle_child(int sig);
void tstp_handler(int sig);

#endif