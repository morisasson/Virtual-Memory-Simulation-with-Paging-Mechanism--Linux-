//208067587
#include "mem_sim.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static void bring_page_into_memory(sim_database* mem_sim, int page_num);

sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_heap_stack_size) {
    sim_database* mem_sim = (sim_database*)malloc(sizeof(sim_database));
    if (!mem_sim) {
        perror("Failed to allocate memory for sim_database");
        return NULL;
    }

    // Initialize page table
    for (int i = 0; i < NUM_OF_PAGES; i++) {
        mem_sim->page_table[i].V = 0;
        mem_sim->page_table[i].D = 0;
        mem_sim->page_table[i].P = (i < (text_size / PAGE_SIZE)) ? 1 : 0;
        mem_sim->page_table[i].frame_swap = -1;
    }

    // Initialize main memory with zeros
    memset(mem_sim->main_memory, '0', MEMORY_SIZE);

    mem_sim->program_fd = open(exe_file_name, O_RDONLY);
    if (mem_sim->program_fd < 0) {
        perror("Failed to open executable file");
        free(mem_sim);
        return NULL;
    }

    mem_sim->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (mem_sim->swapfile_fd < 0) {
        perror("Failed to open or create swap file");
        close(mem_sim->program_fd);
        free(mem_sim);
        return NULL;
    }

    if (ftruncate(mem_sim->swapfile_fd, SWAP_SIZE) != 0) {
        perror("Failed to set swap file size");
        close(mem_sim->program_fd);
        close(mem_sim->swapfile_fd);
        free(mem_sim);
        return NULL;
    }

    // Initialize swap file with zeros
    char zero = '0';
    for (int i = 0; i < SWAP_SIZE; i++) {
        if (write(mem_sim->swapfile_fd, &zero, sizeof(char)) != sizeof(char)) {
            perror("Failed to initialize swap file with zeros");
            close(mem_sim->program_fd);
            close(mem_sim->swapfile_fd);
            free(mem_sim);
            return NULL;
        }
    }

    mem_sim->text_size = text_size;
    mem_sim->data_size = data_size;
    mem_sim->bss_heap_stack_size = bss_heap_stack_size;

    return mem_sim;
}


char load(sim_database* mem_sim, int address) {
    if (address < 0 || address >= (NUM_OF_PAGES * PAGE_SIZE)) {
        fprintf(stderr, "Error: Invalid address\n");
        return '\0';
    }

    int page_num = address / PAGE_SIZE;
    int offset = address % PAGE_SIZE;
    page_descriptor* page = &mem_sim->page_table[page_num];

    if (!page->V) { // Page not in memory
        bring_page_into_memory(mem_sim, page_num);
        if (!page->V) {
            fprintf(stderr, "Error: Page could not be brought into memory\n");
            return '\0';
        }
    }

    int physical_address = page->frame_swap * PAGE_SIZE + offset;
    return mem_sim->main_memory[physical_address];
}

void store(sim_database* mem_sim, int address, char value) {
    if (address < 0 || address >= (NUM_OF_PAGES * PAGE_SIZE)) {
        fprintf(stderr, "Error: Invalid address\n");
        return;
    }

    int page_num = address / PAGE_SIZE;
    int offset = address % PAGE_SIZE;
    page_descriptor* page = &mem_sim->page_table[page_num];

    if (!page->V) { // Page not in memory
        bring_page_into_memory(mem_sim, page_num);
        if (!page->V) {
            fprintf(stderr, "Error: Page could not be brought into memory\n");
            return;
        }
    }

    if (page->P == 1) { // Read-only page
        fprintf(stderr, "Error: Write permission denied\n");
        return;
    }

    int physical_address = page->frame_swap * PAGE_SIZE + offset;
    mem_sim->main_memory[physical_address] = value;
    page->D = 1; // Mark page as dirty
}
static void bring_page_into_memory(sim_database* mem_sim, int page_num) {
    page_descriptor* page = &mem_sim->page_table[page_num];

    if (page->V) return; // Already in memory

    // Find a free frame
    int frame = -1;
    for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++) {
        int used = 0;
        for (int j = 0; j < NUM_OF_PAGES; j++) {
            if (mem_sim->page_table[j].V && mem_sim->page_table[j].frame_swap == i) {
                used = 1;
                break;
            }
        }
        if (!used) {
            frame = i;
            break;
        }
    }

    if (frame == -1) {
        // Need to swap out a page (FIFO)
        for (int j = 0; j < NUM_OF_PAGES; j++) {
            if (mem_sim->page_table[j].V) {
                frame = mem_sim->page_table[j].frame_swap;
                if (mem_sim->page_table[j].D) {
                    // Write back to swap
                    lseek(mem_sim->swapfile_fd, frame * PAGE_SIZE, SEEK_SET);
                    write(mem_sim->swapfile_fd, &mem_sim->main_memory[frame * PAGE_SIZE], PAGE_SIZE);
                }
                mem_sim->page_table[j].V = 0;

                // Zero out the frame in main memory
                memset(&mem_sim->main_memory[frame * PAGE_SIZE], '0', PAGE_SIZE);
                break;
            }
        }
    }

    if (frame == -1) {
        fprintf(stderr, "Error: No free frame found\n");
        return;
    }

    // Load the page into the frame
    if (page->D == 1) {
        // Load from swap file if the page is dirty
        lseek(mem_sim->swapfile_fd, page->frame_swap * PAGE_SIZE, SEEK_SET);
        read(mem_sim->swapfile_fd, &mem_sim->main_memory[frame * PAGE_SIZE], PAGE_SIZE);
        // Clear the page from swap (put zeros)
        lseek(mem_sim->swapfile_fd, page->frame_swap * PAGE_SIZE, SEEK_SET);
        char zero = '0';
        for (int i = 0; i < PAGE_SIZE; i++) {
            write(mem_sim->swapfile_fd, &zero, sizeof(char));
        }
    } else if (page->P == 1 || (page_num <= (mem_sim->text_size + mem_sim->data_size) / PAGE_SIZE)) {
        // Read-only page or text/data page from exec file
        lseek(mem_sim->program_fd, page_num * PAGE_SIZE, SEEK_SET);
        read(mem_sim->program_fd, &mem_sim->main_memory[frame * PAGE_SIZE], PAGE_SIZE);
    } else {
        // New blank page
        memset(&mem_sim->main_memory[frame * PAGE_SIZE], '0', PAGE_SIZE);
    }

    page->frame_swap = frame;
    page->V = 1;
    page->D = 0; // Newly loaded page is not dirty

}


void print_memory(sim_database* mem_sim) {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", mem_sim->main_memory[i]);
    }
}

void print_swap(sim_database* mem_sim) {
    char str[PAGE_SIZE];
    int i;
    printf("\n Swap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
        for(i = 0; i < PAGE_SIZE; i++) {
            printf("[%c]\t", str[i]);
        }
        printf("\n");
    }
}

void print_page_table(sim_database* mem_sim) {
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame_swap\n");
    for(i = 0; i < NUM_OF_PAGES; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V,
               mem_sim->page_table[i].D,
               mem_sim->page_table[i].P, mem_sim->page_table[i].frame_swap);
    }
}

void clear_system(sim_database* mem_sim) {
    close(mem_sim->program_fd);
    close(mem_sim->swapfile_fd);
    free(mem_sim);
}
