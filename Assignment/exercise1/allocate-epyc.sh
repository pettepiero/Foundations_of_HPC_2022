#!/bin/bash

# Command to allocate resources using salloc
salloc -N1 -p EPYC --ntasks-per-node 1 -A dssc --time=01:00:00

