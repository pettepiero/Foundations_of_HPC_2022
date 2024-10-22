#!/bin/bash

# Command to allocate resources using salloc
salloc -N2 -p EPYC --ntasks-per-node 3 -A dssc --time=01:00:00
module load openMPI/4.1.6/gnu/14.2.1
