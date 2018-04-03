#!/bin/bash

if [[ -z "$1" ]]
then
    echo "Usage: timeit [minimum threads number] [maximum threads number] [executable] {parameters}"
	exit
fi

echo "nthreads, s" > results.csv

for (( i = $1; i <= $2; i++ ))
do
	echo "--> nthreads = $i"
	echo > results.tmp
	for ((k = 0; k < 10; k++ ))
	do
		./$3 $i $4 >> results.tmp
	done
	echo "$i, $(python avg.py results.tmp)" >> results.csv
done
rm results.tmp


echo "Done"
