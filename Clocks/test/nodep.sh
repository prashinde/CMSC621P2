#!/bin/bash
tdaemon=4
VALGRIND=
dfile="centralize"
truncate --size 0 $dfile
echo 1 > $dfile
#VALGRIND="valgrind --track-origins=yes --log-file="valg""
for i in {1..4} 
do
	if [ $i -eq $tdaemon ]
	then
		$VALGRIND ../bin/node $i nodelist 1 `shuf -i 100-300 -n 1` $dfile > proc$i 2>&1 &
	else
		$VALGRIND ../bin/node $i nodelist 0 `shuf -i 100-300 -n 1` $dfile > proc$i 2>&1 &
	fi
done
