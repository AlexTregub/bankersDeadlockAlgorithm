# bankersDeadlockAlgorithm
Spring 2024, CS 33211-001, Project 2 - Implementation of Banker’s Algorithm for deadlock avoidance in C. Evaluates safe sequence from file input.

# Banker's Deadlock Avoidance Algorithm Implementation
Banker's deadlock avoidance algorithm can be implemented to ensure that a system will never enter a deadlock, by 'pre-allocating' a request, and then testing to see if there is any sequence of processes that can then be run without running out of available resources. However, this algorithm assumes that once a proccess allocates its 'maximum' amount of resources, it will complete and release all resources in a finite amount of time, there is a constant amount of processes and resources, and that the overhead of the algorithm is acceptable. We also need to be able to know the maximum resources that a process will use before being executed. Due to the overhead cost and the required 'a priori' knowledge needed before scheduling a process this algorithm, the algorithm is not often used (and neither is deadlock avoidance). The implementation of this algorithm uses 4 arrays allocated at runtime: the available resources 1d array, the maximum allocatable resources 2d array per processs, the currently allocated resources 2d array per process, and the remaining needed resources 2d array per process, where the needed resources array is the maximum allocatable resources - currently allocated resources.

My implementation of the algorithm will attempt to find a queue containing all processes in an order such that each can be executed using the available resources, where once complete, they release their allocated resources to the available pool. For each position in the process queue, the algorithm searches from low to high index to select the next process to run allowing processes to be prioritized by being placed higher in the system state table (stored in input file). If, for any position in the queue, no process exists that can be scheduled, then the system is not in a safe state, and further itterations will not be attempted. As no process can have a negative amount of currently allocated resources, the execution order of processes does not matter, as long as there exists some order of processes that can be run, the system is in a safe state and the pre-allocation can be allowed. 

My implementation does not support 'attempting' allocations within the program, however, by passing the program a 'state' file which already has the resources allocated to check if the system would be in a safe state if the allocation were to occur. The return from the program would then have to be parsed from standard output, which is outside of the scope of this project. The program uses malloc to create 2d and 1d arrays of needed size to adapt to different needs, depending on the system state configured in the input file, which is passed as the last argument to the compiled binary. This 'hybrid tab file' is parsed into the arrays, and then processed to determine the 'saftey' of the system state. No configuration is needed for this program outside of creating the input file which stores your system state, and parsing the output from it. 

# Usage
Clone the github repository into a local directory on your system:
```
git clone https://github.com/AlexTregub/bankersDeadlockAlgorithm
```
Compile the downloaded code:
```
bash ./buildScript.sh
```
And run the program either with the included sample data, or with your own input file:
```
# Using included sampleInputFile.txt
bash ./runScript.sh

# Or using your own input file
bash ./runScript.sh ./yourInputFile.something
```
For more advanced output, you can use the script with the 'debug' output, and fully parse the output yourself:
```
## Using the debug script (does not need build 'dependencies' of grep and cat)
# Sample data:
bash ./debugRunScript.sh

# Your data
bash ./debugRunScript.sh ./yourInputFile.something

## Using the compiled binary directly (does not need build 'dependencies' of grep and cat)
./bankersAlgo ./yourInputFile.something
```

# Building Your Own Input File
To create your own input file to check the 'saftey' of a system, you can build off the example provided in this repository:
./sampleInputFile.txt: 
```
5  3        
System's currently allocated resources (n*m)
0  1  0
2  0  0
3  0  2
2  1  1
0  0  2
System maximum allocatable resources (n*m)
7  5  3
3  2  2
9  0  2
2  2  2
4  3  3
System available resources (1*m)
3  3  2
```
The first 2 tab separated files in the file (can have any name) are the process/row count and the resource/column count respectively THESE VALUES ARE USED TO ALLOCATE, AND READ VALUES FROM THE FILE. IF THESE VALUES ARE INACCURATE, THIS PROGRAM CANNOT RUN. This is followed by an newline (\n) And a single comment line. This comment line can contain any text or nothing, however, must end with a newline - thus it can be empty if you do not wnat to store any extra data in the file. The next 2d array will store the currently allocated resources per process (which are rows) with information in the columns (namely the current resource usage of each type). The 2d array is also tab seperated with a newline ending the row. The next line is a comment line, and will be ignored, however you must keep that line in the file - even if empty. The second 2d array (of same size as the first, and of matching size defined by the header) stores the maximum allocatable resources per process (rows) and per resource (columns). This and the currently allocated resources array will be used to calculate the needed resources array. Following the second 2d array, is another comment line (also can contain anything, but must remain), and finally a 1d array of the amount of available resources initially in the system. The file will not be read further.

For all values in these arrays and the header, only positive integers will be accepted. Attempting to input any other type of value will result in undefined behavior. If any of the currently allocated resources per process are larger than the matching process row of the maximum allocatable resources array will result in an invalid state, and the program will not execute. Processes information shall be stored with a matching index in each 2d array (where the first row of the currently allocated resources array 'matches' with the first row of the maximum allocatable resources array). 

# Sample run logs (from local machine)
Example outputs of this program are included in sampleRun.log and sampleDebugRun.log from the executions of './runScript.sh' and './runDebugScript.sh' respectively.

Two seperate 'types' of outputs are provided: a step by step 'simulation' of the system state, which 'highlights' process to be executed at a current step, and then in the next step assumes that the program completed its execution, and updates the available resources. Additionally, the program outputs a 'summary' of the steps of execution, labeled by P#, where # is the index of the process starting from 0 in the input file's 2d arrays. (How the program 'sees' the input file is the first 'simulation' frame, which can help with debugging issues with your input file - visible when system is run from either script)

If the system is in an unsafe state, however, the program will only output the initial 'simulation' frame and the statement 'System is not in a safe state.'. 
