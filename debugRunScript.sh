#!/bin/bash
# Script to run file (with all outputs)
# - Will (if no options passed) run using the provided sampleInputFile, 
#       otherwise if a file is passed as the first argument, will execute
#       Banker's algorithm with the passed file. (Selection handled by binary) 

# Ensure program compiled
if ! test -f ./bankersAlgo; then
    echo "ERROR : './bankersAlgo' does not exist. Run './buildScript.sh' first."
    exit 1;
fi

# Execute passing sample file AND user-provided file.
./bankersAlgo ./sampleInputFile.txt $1
