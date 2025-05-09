
#ifndef SPAWN_PROCESSES_H
#define SPAWN_PROCESSES_H

#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

void bring_foreground_from_background(string instruct);
void bring_stop_to_background(string instruct);
int spawn_processes(string instruction);

#endif