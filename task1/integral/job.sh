#!/bin/bash

#PBS -l walltime=00:01:00,nodes=4:ppn=4
#PBS -N s57304_job
#PBS -q batch

cd $PBS_O_WORKDIR
mpirun --hostfile $PBS_NODEFILE -np $NPROC $EXECNAME $PARAMS
