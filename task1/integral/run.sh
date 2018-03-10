#!/bin/bash

if [[ -z $1 ]]
then
	echo "Usage: run [nproc] [execname] [params]"
	exit
fi

qsub job.sh -v NPROC=$1,EXECNAME=$2,PARAMS=$3
