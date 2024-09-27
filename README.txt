//208067587

# Virtual Memory Simulation
By Mori Sason

## Overview
This program simulates the virtual memory management of a computer using a paging mechanism. The simulation includes functions to load and store data in memory, print the current state of the memory, swap file, and page table, and clear the system resources upon completion.

## Files
- `mem_sim.h`: Header file containing the function signatures and structure definitions.
- `mem_sim.c`: Implementation of the functions defined in `mem_sim.h`.
- `main.c`: A small main program to demonstrate the use of the functions.
- `README.md`: This file, providing an overview of the system, files, functions, and how to compile and run the program.
- `run_me.sh`: Shell script to compile and run the program.

## Functions
### `init_system`
Initializes the simulation environment, including opening the executable and swap files, initializing the page table, and setting up the main memory and swap file.

### `load`
Loads a specific logical address for reading, ensuring the relevant page is in main memory.

### `store`
Stores a value at a specific logical address for writing, ensuring the relevant page is in main memory.

### `print_memory`
Prints the contents of the main memory.

### `print_swap`
Prints the contents of the swap file.

### `print_page_table`
Prints the current state of the page table.

### `clear_system`
Closes open files and frees dynamic memory.

## Compilation and Execution
To compile and run the program, execute the following command in the terminal:
```sh
./run_me.sh

License
-------

This project is licensed under the MIT License. See the LICENSE file for more details.


Author
------

Mori Sasson  
LinkedIn: https://www.linkedin.com/in/mori-sason-9a4811281  
Email: 8mori8@gmail.com


