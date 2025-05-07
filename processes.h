#ifndef PROCESSES_H
#define PROCESSES_H

int detect_background(char* instruct);
int spawn_processes(process_map* processes, char* instruct, pid_t child_pid);



#endif