#ifndef PROCESS_MAP_H
#define PROCESS_MAP_H


typedef struct process_map {
    int process_map[1000][3];
    int size;
} process_map;

process_map* new_process_map();
int get_id_by_order(process_map* processes, int order);
int get_order_by_id(process_map* processes, pid_t process_id);
void add_pid_to_map(process_map* processes, pid_t process_id, int is_foreground);
int remove_pid_map(process_map* processes, pid_t process_id);
void print_process(process_map* processes);

#endif