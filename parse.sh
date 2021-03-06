#!/bin/bash

rm *.stat
for i in {0..19}
do 
	echo "-------------------------------"
	echo Round=$i
	#consumers ending interest
	#grep "^$i\." log.txt | grep -c '0 ndn.fw:OnInterest\|1 ndn.fw:OnInterest\|2 ndn.fw:OnInterest\|3 ndn.fw:OnInterest'
	for consumernodeid in {0..3}
	do
		grep "^$i\." log.txt | grep -c "$consumernodeid ndn.fw:OnInterest"
		grep "^$i\." log.txt | grep "$consumernodeid ndn.fw:OnInterest" >> "consumernodeid-"$consumernodeid."timestamp-"$i.stat
	done
	#grep "^$i\." log.txt | grep '[0-3] ndn.fw:OnInterest'
	#producers receiving interest
	grep "^$i\." log.txt | grep -c '[5-9] ndn.fw:OnInterest\|[1-9][1-9] ndn.fw:OnInterest'
done
