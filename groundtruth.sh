#!/bin/bash

for i in {0..3}
do
	echo "Consumer "$i
	echo "------------------------------"	
	for j in {1..10}
	do
		a=$(grep -c "/producer"$j consumernodeid-$i*|awk -F: '{s+=$2} END {print s}')
		b=$(grep -c "/producer" consumernodeid-$i*|awk -F: '{s+=$2} END {print s}')
		
		echo "Producer "$j"  " $a 
		echo "$a $b"|awk '{printf "Proportion of total interests issued by consumer:%.3f \n", $1/$2}'
					
	done
	echo "Total no. of interests by consumer "$i": "$b
	echo "=================================="
done	
