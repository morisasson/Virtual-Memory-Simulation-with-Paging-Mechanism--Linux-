//208067587
#ifndef MEM_SIM_H
#define MEM_SIM_H

#include <stdio.h>

#define PAGE_SIZE 8
#define NUM_OF_PAGES 25
#define MEMORY_SIZE 40
#define SWAP_SIZE 200

typedef struct page_descriptor {
    unsigned int V; // valid
    unsigned int D; // dirty
    unsigned int P; // permission
    unsigned int frame_swap; // frame or swap number
} page_descriptor;

typedef struct sim_database {
    page_descriptor page_table[NUM_OF_PAGES];
    int swapfile_fd;
    int program_fd;
    char main_memory[MEMORY_SIZE];
    int text_size;
    int data_size;
    int bss_heap_stack_size;
} sim_database;

sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size);
char load(sim_database* mem_sim, int address);
void store(sim_database* mem_sim, int address, char value);
void print_memory(sim_database* mem_sim);
void print_swap(sim_database* mem_sim);
void print_page_table(sim_database* mem_sim);
void clear_system(sim_database* mem_sim);

#endif // MEM_SIM_H

