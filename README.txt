Here lies a command line interface using system calls to simulate a pseudo-shell, and a master control program which not only spawns and executes processes, but manages them via signal handling and process scheduling (round robin). Both programs are written entirely in C and include a MakeFile. Example input files are provided.

*** NOTE: This code was developed in a Linux environment using a VM, and is optimized to run only on a Linux OS. ***


MCP: 

You will notice in input1.txt reference to executables called "cpubound" and "inbound". I provided these files in the repository, feel free to execute and use them for input1. If you do not wish to do so, use input2.txt. 

Part 1: Reading and executing processes from a file 
Part 2: Implements signals to start and stop running processes
Part 3: Uses round robin technique to schedule running processes so they receive "equal" share of a CPU (1 second time quantum) 
Part 4: Implements a "top" like system of showing program data. 

Pseudo-Shell: 

This program supports two modes: File Mode and Command Mode. File Mode takes commands as input from a file. Command Mode takes commands from the command line, commands and parameters may be separated using a semicolon. 