#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

typedef struct process_map {
    int process_map[1000][3]; // { [ Order, pid, type ], ...       }
    int size;
} process_map;


process_map* new_process_map(){
    process_map* processes = malloc(sizeof(process_map));
    processes->size = 1;
    return processes;
}

int get_id_by_order(process_map* processes, int order){
    int process_id = -1; // not found.
    if (processes->process_map[order][0] != 0){
        process_id = processes->process_map[order][1];
    }
    return process_id;
}

int get_order_by_id(process_map* processes, pid_t process_id){ // pass in pid, returns order.
    int process_order = -1; // -1 is not found.
    for (int i = 0; i < processes->size; i++){
        if (processes->process_map[i][1] == process_id){
            process_order = processes->process_map[i][0];
            break;
        }
    }
    return process_order;
}

void add_pid_to_map(process_map* processes, pid_t process_id, int is_foreground){
    int index = processes->size-1;
    processes->process_map[index][0] = processes->size;
    processes->process_map[index][1] = process_id;
    processes->process_map[index][2] = is_foreground;
    processes->size+=1; 
}

int remove_pid_map(process_map* processes, pid_t process_id){
    int return_val;
    for (int i = 0; i < processes->size; i++){
        if (processes->process_map[i][1] == process_id){
            return_val = processes->process_map[i][0];
            for (int j = i; j < processes->size-1; j++){
                processes->process_map[j][0] = processes->process_map[j+1][0];
                processes->process_map[j][1] = processes->process_map[j+1][1];
                processes->process_map[j][2] = processes->process_map[j+1][2];
            }
            processes->size--;
            return return_val;
        }
    }
    return -1; // not found.
}

void print_process(process_map* processes){
    for (int i = 0; i<(processes->size)-1; i++){
        printf("[%d] Running\n", processes->process_map[i][0]);
    }
}
