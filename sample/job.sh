#!/bin/bash

#PBS -l walltime=00:00:05,nodes=1:ppn=3
#PBS -N prac1
#PBS -q batch

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np 3 ./sample.out
