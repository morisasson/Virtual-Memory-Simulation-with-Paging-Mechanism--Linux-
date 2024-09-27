#!/bin/bash
# Compile the program
gcc mem_sim.c main.c -o mem_sim -Wall

# Check if compilation was successful
if [ $? -eq 0 ]; then
    # Run the program
    ./mem_sim
else
    echo "Compilation failed."
fi

