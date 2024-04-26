#!/bin/bash
# Dependancy check (for ./runScript, need grep and cut)
echo "Dependancies needed (for ./runScript): "
test=true;
if ! (which cut); then
    echo "WARNING : Missing 'cut' package. './runScript.sh' will not work, only './debugScript.sh' will execute.";
    test=false;
fi
if ! (which grep); then
    echo "WARNING : Missing 'grep' package. './runScript.sh' will not work, only './debugScript.sh' will execute.";
    test=false;
fi 

# Print status of dependancy check
if $test; then
    echo "Dependancies exist!"
fi

# Compile bankersAlgorithm program to binary (WILL NOT EXECUTE FILE)
if (gcc bankersAlgo.c -o bankersAlgo); then
    echo "Compiled to ./bankersAlgo"
fi

# Standard file execution done via:
# ./bankersAlgo ./sampleInputFile.txt | grep OUTPUT | cut -d ':' -f 2
# With all debug information via:
# ./bankersAlgo ./sampleInputFile.txt