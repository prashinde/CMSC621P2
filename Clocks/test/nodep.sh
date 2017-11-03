#!/bin/bash
tdaemon=4
VALGRIND=
dfile="centralize"
causality=1
truncate --size 0 $dfile
echo 1 > $dfile
VALGRIND="valgrind --track-origins=yes --log-file="valg""
for i in {1..4} 
do
	if [ $i -eq $tdaemon ]
	then
		$VALGRIND$i ../bin/node $i nodelist 1 `shuf -i 100-300 -n 1` $causality 100 $dfile > proc$i 2>&1 &
	else
		$VALGRIND$i ../bin/node $i nodelist 0 `shuf -i 100-300 -n 1` $causality 100 $dfile > proc$i 2>&1 &
	fi
done
