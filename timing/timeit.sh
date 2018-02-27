#!/bin/bash

if [[ -z "$1" ]]
then
    echo "Usage: timeit [minimum process number] [maximum process number] [executable] {parameters}"
	exit
fi

for (( i = $1; i <= $2; i++ ))
do
	echo "Loading NP $i..."
	TEMP="$(qsub job.sh -v NPROC=$i,EXECNAME=$3,PARAMS=$4 2>/dev/null)"
	while [[ -z $TEMP ]]
	do
		#echo "Queue is busy, waiting..."
		sleep 1
		TEMP="$(qsub job.sh -v NPROC=$i,EXECNAME=$3,PARAMS=$4 2>/dev/null)"

	done
	echo "OK"
	RES_NAME="$i.tmp"
	touch $RES_NAME.wait
	./waiter.sh $TEMP $RES_NAME &
done

#Accumulating timings
ACCUM="results.csv"
echo "np, ms" > $ACCUM

for (( i = $1; i <= $2; i++ ))
do
	echo "Waiting for NP $i to finish..."
	RES_NAME="$i.tmp"
	while [ -f $RES_NAME.wait ]
	do
		sleep 1
	done
	echo "OK"
	echo "$i, $(cat $RES_NAME)" >> $ACCUM	
	rm $RES_NAME
done

echo "Done"
