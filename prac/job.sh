#!/bin/bash

#PBS -l walltime=00:02:00,nodes=1:ppn=1
#PBS -N timing
#PBS -q batch

cd $PBS_O_WORKDIR
for ((i = 0; i < 20; i++ ))
do
	mpirun --hostfile $PBS_NODEFILE -np $NPROC $EXECNAME $PARAMS
done
