#!/bin/bash

# Command to allocate resources using salloc
salloc -N1 -p THIN --ntasks-per-node 1 -A dssc --time=01:00:00
openMPI/4.1.6/gnu/14.2.1
