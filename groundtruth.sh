#!/bin/bash

for i in {0..3}
do
	echo "Consumer "$i
	echo "------------------------------"	
	sum=0
	for j in {1..10}
	do
		a=$(grep -c "/producer"$j"/" consumernodeid-$i*|awk -F: '{s+=$2} END {print s}')
		b=$(grep -c "/producer" consumernodeid-$i*|awk -F: '{s+=$2} END {print s}')
		
		echo "Producer "$j"  " $a 
		echo "$a $b"|awk '{printf "proportion of total interests issued by consumer:%.3f \n", $1/$2}'
		sum=$(($sum+$a))
					
	done
	#echo $sum
	echo "Total no. of interests by consumer "$i": "$b
	echo "$sum $b" |awk '{printf "sum :%.3f \n",$1/$2}'
	echo "=================================="
done			
