#!/bin/bash
tdaemon=3
VALGRIND=
#VALGRIND="valgrind --log-file="valg""
for i in {1..3} 
do
	if [ $i -eq $tdaemon ]
	then
		$VALGRIND ../bin/node $i nodelist 1 `shuf -i 100-300 -n 1`  > proc$i 2>&1 &
	else
		$VALGRIND ../bin/node $i nodelist 0 `shuf -i 100-300 -n 1`  > proc$i 2>&1 &
	fi
done
