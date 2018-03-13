#!/bin/bash

#PBS -l walltime=00:05:00,nodes=4:ppn=4
#PBS -N timing
#PBS -q batch

cd $PBS_O_WORKDIR
for ((i = 0; i < 10; i++ ))
do
	mpirun --hostfile $PBS_NODEFILE -np $NPROC $EXECNAME $PARAMS
done
