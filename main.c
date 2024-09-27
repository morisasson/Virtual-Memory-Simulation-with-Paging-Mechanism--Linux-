//208067587
#include "mem_sim.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    sim_database* mem_sim = init_system ("exec_file", "swap_file" ,40,40,120);
    if(mem_sim == NULL){
        exit(EXIT_FAILURE);
    }


    store(mem_sim, 40,  '1');
    store(mem_sim, 50,  '2');
    store(mem_sim, 60,  '3');
    store(mem_sim, 70,  '4');
    store(mem_sim, 75,  '5');
   store(mem_sim, 80 , '6');
    store(mem_sim, 90 , '7');
   store(mem_sim, 51 , '9');

    print_memory(mem_sim);
    print_page_table(mem_sim);
    print_swap(mem_sim);
    clear_system(mem_sim);
    return 0;
}
