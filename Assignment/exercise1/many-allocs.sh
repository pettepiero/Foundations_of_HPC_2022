#!/bin/bash

# Command to allocate resources using salloc
salloc -N3 -p THIN --ntasks-per-node 2 -A dssc --time=01:00:00
module load openMPI/4.1.6/gnu/14.2.1
