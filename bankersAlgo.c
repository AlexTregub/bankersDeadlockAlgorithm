///// Alex Tregub
///// CS33211-001
///// Banker's Algorithm for Deadlock Avoidance
///// ===========
///// v1.2.3
///// For fixed number of processes, and fixed amount of resources, given a
/////     system state (via file), will attempt to find a 'safe' allocation
/////     sequence for the current system state. If found, will printf the 
/////     sequence, otherwise will print that system is in unsafe state.
///// - Compile via > gcc bankersAlgo.c -o bankersAlgo
///// - Run via > ./bankerAlgo ./INPUT_FILE.txt
///// - Clean output (without debug messages/tags) > ./bankerAlgo ./INPUT_FILE.txt | grep OUTPUT | cut -d ':' -f 2
///// ===========
///// - Fixed header parsing not leaving good state for next parser
///// - Added 'OUTPUT : ' prefix to all 'standard' program outputs for consistency. Grep for 'OUTPUT'/'INFO'/'ERROR' for easier sorting. 
///// - In future versions, use struct/class to store system state for more managable useage
#include <stdio.h>
#include <stdlib.h>

#define MAX_PTR_ALLOCS_COUNT 8
#define FILES_USED 1

int *PTR_ALLOC_RES[MAX_PTR_ALLOCS_COUNT] = {0}; // Global array to store pointers used in program

int allocInit2dIntArray(int,int,int,int**,int,int); // Allocates 2d array. Params: rows,cols,initValue,globalResources,globalResourceCount,globalFilesUsed. Returns index to globalResources array (resolve type when calling ptr)
int allocInit1dIntArray(int,int,int**,int,int); // Allocates 1d array. Params: cols,initValue,globalResources,globalResourceCount,globalFilesUsed. Returns index to globalResources array 
void skipReadLine(FILE*); // Skips forward 1 line in file. Takes: file ptr (from fopen(...))
int readTabSep2dArray(FILE*,int**,int,int); // Reads tab-separated 2d array of known size. Params: file ptr,outputArray,rows,cols. Returns invalidReadFlag (0 if success,-1 if fail)
int readTabSep1dArray(FILE*,int*,int); // Reads tab-separated 1d array of known size. Params: file ptr,outputArray,cols. Returns invalidReadFlag (0 if success, -1 if fail)
int subtract2dArrays(int**,int**,int**,int,int); // Subtracts 2 arrays (1st array - 2nd array) and stores result in output array of known size. Params: outputArray,firstArray,secondArray,rows,cols. Returns resultsNegative (0 if all values in array positive, -1 if not)
void cleanupAllocs(int**,int,int,int); // Cleans up any allocated memory stored in array of ptrs. Params: globalResources,globalResourceCount,globalFilesUsed,toDealloc(-1 if all)
void copy1dArray(int*,int*,int); // Copys values from 1d array into another. Params: resultArray,sourceArray,cols
void printCurrentState(int**,int**,int**,int*,int,int,int); // Prints out state of system stored in multiple arrays. Params: maxAllocatable,isAllocated,needsAllocation,availableResources,rows,cols,highlightProccess(-1 if none)



int main(int argc, char *argv[]) {
    printf("INFO : Bankers Algo version 1.2.3\n");
    
    //// Open file + read header
    FILE *inputFile = fopen(argv[argc-1], "r");
    if (inputFile == NULL) { // If file could not be opened, exit
        printf("ERROR : File (%s) does not exist or cannot be opened.\n",argv[argc-1]); 
        return 1;
    }
    PTR_ALLOC_RES[0] = (int*)inputFile; // Store file ptr as int ptr in first index (within known file ptrs)
    printf("INFO : Opened file (%s) for reading...\n",argv[argc-1]);

    // Parse header information, and store for allocation of arrays (done without dynamic arrays for simplicity)
    int resCount = 0; 
    int procCount = 0;
    fscanf(inputFile,"%d %d",&procCount,&resCount); // Header stored as process count, resource count + eats newline (for consistency)
    if (resCount <= 0 | procCount <= 0) { // If any invalid values are read from header, exit 
        printf("ERROR : File (%s) has an invalid header format. Cannot continue.\n",argv[argc-1]); 
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); 
        return 1; 
    }
    char tempChar = fgetc(inputFile); // Skip past newline for parsing to continue
    printf("INFO : File (%s) has valid header: Rows/Proccess=%i, Cols/Resources=%i\n",argv[argc-1],procCount,resCount);



    //// Allocate arrays - if any allocations fail, exit gracefully
    // Allocate 'currently allocated resources' array
    int targetPtr = allocInit2dIntArray(procCount,resCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Could not allocate %i x %i array for input (for currently used resources).\n",resCount,procCount);
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    int **procAlloc = (int**)PTR_ALLOC_RES[targetPtr]; // Stores allocated 2d ptr (from global resource array)
    
    // Allocate 'maximum allocatable resources' array
    targetPtr = allocInit2dIntArray(procCount,resCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Could not allocate %i x %i array for input (for maximum usable resources).\n",resCount,procCount);
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    int **maxAlloc = (int**)PTR_ALLOC_RES[targetPtr]; // Stores allocated 2d ptr (from global resource array)
    
    // Allocate 'needed resources' array
    targetPtr = allocInit2dIntArray(procCount,resCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Could not allocate %i x %i array for input (for needed resources).\n",resCount,procCount);
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    int **needAlloc = (int**)PTR_ALLOC_RES[targetPtr]; // Stores allocated 2d ptr (from global resource array)
    
    // Allocate 'available resources' array
    targetPtr = allocInit1dIntArray(procCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Could not allocate 1 x %i array for input (for available resources).\n",resCount);
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    int *availRes = PTR_ALLOC_RES[targetPtr]; // Stores allocated 1d ptr (from global resource array)



    //// Parse data into file (tab separated, but shouldn't matter for fscanf)
    skipReadLine(inputFile); // Ignore 1 line of file - after header (no data stored)

    // Read allocated resources array
    if (readTabSep2dArray(inputFile,procAlloc,procCount,resCount)) { // Read data from file into array, if reads failed, exit.
        printf("ERROR : Invalid read of currently allocated resources from file.\n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    skipReadLine(inputFile); // Ignore 1 line of file - after array

    // Read maximum allocatable resources array
    if (readTabSep2dArray(inputFile,maxAlloc,procCount,resCount)) { // Read data from file into array, if reads failed, exit.
        printf("ERROR : Invalid read of currently allocated resources from file.\n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    skipReadLine(inputFile); // Ignore 1 line of file - after array

    // Read available resources array
    if (readTabSep1dArray(inputFile,availRes,resCount)) { // Attempt to read available resources array, should be final entry in file
        printf("ERROR : Invalid read of available resources from file.\n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }
    cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,0); // Deallocate ONLY opened file (no longer needed)

    // Set up 'needed' array
    if (subtract2dArrays(needAlloc,maxAlloc,procAlloc,procCount,resCount)) { // Subtract procAlloc from maxAlloc, check for any negative values and store in needAlloc
        printf("ERROR : Invalid result for needed resource array calculation.\n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
        return 1;
    }



    //// Bankers algorithm implementation (Processes queued earlier if position in table is higher)
    // Allocate 1d array for proccesses' queue status (0 if not in queue, 1 if is in queue)
    targetPtr = allocInit1dIntArray(procCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate 1d array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Unable to allocate array1 for Banker's algorithm. \n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); 
        return 1; 
    } // If allocation fails, exit
    int *procCheck = PTR_ALLOC_RES[targetPtr]; // Stores allocated 1d ptr (from global resource array)

    // Allocate 1d array for to-execute queue
    targetPtr = allocInit1dIntArray(procCount,-1,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate 1d array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Unable to allocate array2 for Banker's algorithm. \n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); 
        return 1; 
    } // If allocation fails, exit
    int *procOrder = PTR_ALLOC_RES[targetPtr]; // Stores allocated 1d ptr (from global resource array)

    // Allocate 1d Array for temporary processing + copy 
    targetPtr = allocInit1dIntArray(resCount,0,PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED); // Attempts to allocate 1d array
    if (targetPtr == -1) { // Checks if allocation succeeded
        printf("ERROR : Unable to allocate array for copy of available resource array. \n");
        cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1);
        return 1;
    } // If allocation fails, exit 
    int *availCopy = PTR_ALLOC_RES[targetPtr]; // Stores allocated 1d ptr (from global resource array)
    copy1dArray(availCopy,availRes,resCount); // Copy array to be used for test-operations (to allow for step-by-step state output later)




    // Bankers Algo, exits immediately if system is in unsafe state and cannot queue any processes, preventing unneeded itterations
    //     Due to how algorithm is implemented here, proccesses with lower indexes (P0,P1,...) will be 
    //     queued before processes with higher indexes (P100,P99,...), thus adding a basic 'process queuing priority'.
    //     Not required by project description, however kept for possible use cases. 
    //         - Moving rows/Changing proccess order has very little overhead in the 2d arrays due to the per-row ptr
    //           implementation for them. Eg: Swapping queuing priority of proccess i with proccess j - 
    /*
        int *tempRowPtr = array[i]; 
        array[i] = array[j];
        array[j] = tempRowPtr;
    */
    for (int i = 0; i < procCount; ++i) { // For each 'target' position in queue
        for (int j = 0; j < procCount; ++j) { // For each possible process FOR a position in queue
            if (procCheck[j] == 0) { // If process CAN be selected, check if enough available resources exist
                // If no needed resources > than available, ...
                int canBeQueued = 1; 
                for (int k = 0; k < resCount; ++k) { // Check if any resource needs exceeds available resources
                    if (needAlloc[j][k] > availCopy[k]) { canBeQueued = 0; break; } 
                }

                // ..., queue proccess.
                if (canBeQueued) { 
                    procOrder[i] = j; // Proccess 'j' has passed tests, and can be added to queue (in pos i)
                    for (int k = 0; k < resCount; ++k) {
                        availCopy[k] += procAlloc[j][k]; // Increases available resources, as selected process will have completed before queue proceeds
                    }
                    procCheck[j] = 1; // Flag process as checked

                    break; // Exits out of process checking, and selects next i to check for
                }
            }
        }

        if (procOrder[i] == -1) { // All processes were checked for ith position in queue, and none could be queued.
            printf("OUTPUT : Initial System State:\n");
            printCurrentState(maxAlloc,procAlloc,needAlloc,availRes,procCount,resCount,-1); // Output system current state
            
            printf("OUTPUT : System is not in a safe state.\n");
            cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
            return 0;
        }
    }



    //// Output safe sequence results (unsafe result would have returned already)
    printf("OUTPUT : Initial System State:\n");
    printCurrentState(maxAlloc,procAlloc,needAlloc,availRes,procCount,resCount,-1); // Output system current state

    for (int i = 0; i < procCount; ++i) { // Updates states based on queue: display state first (with queued proccess selected), and THEN update values
        printf("OUTPUT : Exec %i, System state:\n",i+1); // Output 'real' execution order
        printCurrentState(maxAlloc,procAlloc,needAlloc,availRes,procCount,resCount,procOrder[i]); // Output system current state (pre-update)
        
        for (int j = 0; j < resCount; ++j) { // Updating values in arrays
            availRes[j] += procAlloc[procOrder[i]][j]; // Updates the avail-resources for proccess which is in the ith position in queue
            procAlloc[procOrder[i]][j] = 0; // Sets the new 'currently' allocated resources to 0
            needAlloc[procOrder[i]][j] = 0; // No longer needs any allocated resources, process has completed
            maxAlloc[procOrder[i]][j] = 0; // Process no longer running, thus dropped
        }
    }
    printf("OUTPUT : Final System State:\n");
    printCurrentState(maxAlloc,procAlloc,needAlloc,availRes,procCount,resCount,-1); // Final system state displayed (available resources updated, all other processes zeroed)
    //printf("\n");    



    // Outputs safe sequence summary for system state 
    printf("OUTPUT : Safe sequence displayed:\n");
    printf("OUTPUT : "); // printf 'tag' for sorting of program output
    for (int i = 0; i < procCount-1; ++i) {
        printf("P%i > ",procOrder[i]);
    }
    printf("P%i\n",procOrder[procCount-1]);

    //// Cleanup and exit. Algorithm succeeded
    cleanupAllocs(PTR_ALLOC_RES,MAX_PTR_ALLOCS_COUNT,FILES_USED,-1); // Deallocate all currently allocated resources
    return 0;
}



//// Function definitions
int allocInit2dIntArray(int rowCount,int colCount, int defaultVal, int **resources, int resourceCount, int filesUsed) { // Function to allocate and initialize 'true' 2d array. Access via name[i][j]. Returns index in resAlloc array that is passed. 
    // Ensure non-zero allocation
    if (rowCount <= 0 || colCount <= 0) { return -1; }
    
    // Allocate memory (if possible), otherwise exit. Handle error later
    int arrayMemSize = sizeof(int *)*(rowCount) + sizeof(int)*(colCount)*(rowCount); // Size = row*col ints + row ptrs for access with 2 [] operators
    int **new2dArray = (int **)malloc(arrayMemSize);
    if (new2dArray == NULL) { return -1; } // If malloc fails do not attempt to access non-real addrs.

    // Setup array + initialize
    int *tempPtr = (int *)(new2dArray + rowCount); // Point to first element of (currently 1d) array
    for (int i = 0; i < rowCount; ++i) { // Setup internal array ptrs
        new2dArray[i] = (tempPtr + colCount*i); // Per-row offsets used
    }

    for (int i = 0; i < rowCount; ++i) { // Per-row ...
        for (int j = 0; j < colCount; ++j) { // Per-col ...
            new2dArray[i][j] = defaultVal; // Initialize values to 0
        }
    }

    // Store ptr in resources array for program to access
    int posInResArr = -1;
    for (int i = filesUsed; i < resourceCount; ++i) { // Find available position in array
        if (resources[i] == NULL) { posInResArr = i; break; }
    }
    if (posInResArr == -1) { // Did not find position to store ptr in
        free(new2dArray); // Free to prevent memory leak
        return posInResArr; // Return -1 to indicate error
    }

    // Pass back initialized array
    resources[posInResArr] = (int*)new2dArray; // Ensure correct ptr type stored, upto user to ensure correct type used when this is re-cast to original type (int**)
    return posInResArr;
}
int allocInit1dIntArray(int valCount, int defaultVal, int **resources, int resourceCount, int filesUsed) { // Function to allocate and initialize 1d arrays. Returns index in resAlloc array that is passed
    // Ensure non-zero allocation
    if (valCount <= 0) { return -1; }

    // Allocate memory (if possible), otherwise exit. Handle error later
    int *new1dArray = (int *)malloc(valCount*sizeof(int));
    if (new1dArray == NULL) { return -1; } // If malloc fails do not attempt to access non-real addrs.

    // Initialize + Return array
    for (int i = 0; i < valCount; ++i) {
        new1dArray[i] = defaultVal;
    }
    
    // Store ptr in resources array for program to access
    int posInResArr = -1;
    for (int i = filesUsed; i < resourceCount; ++i) { // Find available position in array
        if (resources[i] == NULL) { posInResArr = i; break; }
    }
    if (posInResArr == -1) { // Did not find position to store ptr in
        free(new1dArray); // Free to prevent memory leak
        return posInResArr; // Return -1 to indicate error
    }

    resources[posInResArr] = new1dArray; // Already correct ptr type for array, no need for type conversion
    return posInResArr;
}
void skipReadLine(FILE* file) { // Skips a line (ending with \n)
    char c = ' '; // Discardable char
    do { // Gets next char -> checks if its a newline
        c = fgetc(file); 
    } while (c != '\n'); // Once newline, exits. Next read is past newline
}
int readTabSep2dArray(FILE* file, int ** dataArray, int rowCount, int colCount) { // Read tab separated array into 2d array of known size
    int invalidReadFlag = 0; // Set to 1 if invalid read
    int val = -1; // Value should only need to be 'created' once
    for (int i = 0; i < (rowCount*colCount); ++i) { // Read rows*cols values from file, fscanf handles newlines and tabs automatically
        val = -1; // Invalid reads will show up CLEARLY as -1
        
        fscanf(file," %d",&val); // Read any whitespace and then integer (%d used to ensure base 10)
        if (val < 0) { // Invalid read detected, will not continue reading and will return. (Should be no un-set values in array, thus partitial read ok)
            invalidReadFlag = 1; 
            return invalidReadFlag; // Used for readability, could return const int
        } 
        dataArray[(int)(i/colCount)][i%colCount] = val; // Sets value into array
    }
    char c = fgetc(file); // Skips char after table fully read (consumes newline after final value)
    
    return invalidReadFlag; // Values stored in array already, flag used for checking read success/fail
}
int readTabSep1dArray(FILE* file, int * dataArray, int valCount) { // Read tab separated array into 1d array of known size
    int invalidReadFlag = 0; // Set to 1 if invalid read
    int val = -1; // Value should only need to be 'created' once
    for (int i = 0; i < valCount; ++i) {
        val = -1; // Invalid reads will show up CLEARLY as -1
        fscanf(file, " %d", &val); // Read any whitespace and then integer (%d used to ensure base 10)
        if (val < 0) { // Invalid read detected, will not continue reading and will return. (Should be no un-set values in array, thus partitial read ok)
            invalidReadFlag = 1; 
            return invalidReadFlag;
        } 
        dataArray[i] = val; // Sets value into array
    }
    char c = fgetc(file); // Skips char after table fully read (consumes newline after final value)
    
    return invalidReadFlag; // Values stored in array already, flag used for checking read success/fail
}
int subtract2dArrays(int ** result, int ** firstArr, int ** secondArr, int rowCount, int colCount) { // Subtracts second Array from first Array and stores it in result (known,same size). Returns 1 if any negative value exists
    int resultValNegative = 0; // Flag for any negative values (convinence for this program, not needed otherwise)
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < colCount; ++j) {
            result[i][j] = firstArr[i][j] - secondArr[i][j]; // Component-wise subtraction
            if (result[i][j] < 0) { resultValNegative = 1; } // Checks if any negative values were stored
        }
    }
    return resultValNegative;
}
void cleanupAllocs(int **resources, int resourceCount, int filesUsed, int toDealloc) { // General function to cleanup CURRENTLY allocated resources
    if (toDealloc == -1) { // Deallocate ALL currently available resources
        for (int i = 0; i < resourceCount; ++i) { // Loops through all ptrs
            if (i < filesUsed) { // Ptrs from fopen()
                if (resources[i] != NULL) { // Ensures resource is allocated
                    fclose((FILE*)resources[i]);
                    resources[i] = NULL;
                }
            } else { // Ptrs from malloc()
                free(resources[i]); // Dealloc. free(NULL) does nothing, no need for check - C99_7.20.3.2
                resources[i] = NULL; // Mark as deallocated
            }
        }
        return; // Returns immediately
    }
    
    // Deallocating SPECIFIC FILE resource
    if (toDealloc < filesUsed) { // If ptr to dealloc is a file (opened with fopen())
        if (resources[toDealloc] == NULL) { return; } // Already deallocated 
        
        fclose((FILE*)resources[toDealloc]); // Deallocates target file
        resources[toDealloc] = NULL; // Marks position as deallocated
        return; // Returns, resource deallocated
    }

    // Deallocating SPECIFIC MALLOC resource
    free(resources[toDealloc]); // Free memory at ptr. free(NULL) does nothing - C99_7.20.3.2
    resources[toDealloc] = NULL; // Mark as deallocated
    return;
}
void copy1dArray(int *targetArray, int *sourceArray, int size) { // Copy contents of 1d array to another (of known size)
    for (int i = 0; i < size; ++i) { // Copy all elements of 1d array
        targetArray[i] = sourceArray[i]; 
    }
}
void printCurrentState(int **maximumAllocatable,int **currentlyAllocated,int **neededAllocation,int *availableResources, int rowCount, int colCount, int selProccess) { // Uses printf to output current system state (defined by passed arrays) If selected proccess = -1, none highlighted
    // Arbitrary printf output of current system state, as stored in passed array ptrs
    printf("OUTPUT : P# |  Max  | Alloc | Needs | Avail\n");
    for (int i = 0; i < rowCount; ++i) { // Loop through all rows of arrays
        // Highlighted proccess check (adds emphasis on proccess to-be-executed)
        if (i == selProccess) { // Outputs start of each line as needed, with printf 'tag' 
            printf("OUTPUT : >> P%i | ",i); // If proccess selected, will be slightly offset from other proccesses in state snapshots
        } else {
            printf("OUTPUT : P%i | ",i); 
        }
        
        // Output primary arrays for every line
        for (int j = 0; j < colCount; ++j) { // Print all columns for max...
            printf("%i ",maximumAllocatable[i][j]);
        }
        printf("| ");
        for (int j = 0; j < colCount; ++j) { // Print all columns for current...
            printf("%i ",currentlyAllocated[i][j]);
        }
        printf("| ");
        for (int j = 0; j < colCount; ++j) { // Print all columns for needed...
            printf("%i ",neededAllocation[i][j]);
        }
        printf("| "); // Spacer for values

        // For first line of data, also output 'available' resources
        if (i == 0) {
            for (int j = 0; j < colCount; ++j) {
                printf("%i ",availableResources[j]); // Output Avail resources
            }
        }
        printf("\n"); // Terminate line 
    }
}