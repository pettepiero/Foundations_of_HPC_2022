#!/bin/bash

cd /u/dssc/ppette00/materials/hpc/Assignment/exercise1/test/
srun make
cd ..
srun build/Testdynamics.x
 
