#!/bin/bash
tdaemon=3
for i in {1..3} 
do
	if [ $i -eq $tdaemon ]
	then
		../bin/node $i nodelist 1 `shuf -i 100-300 -n 1` &
	else
		../bin/node $i nodelist 0 `shuf -i 100-300 -n 1` &
	fi
done
