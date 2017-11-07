#!/bin/bash
tdaemon=4
VALGRIND=
dfile="centralize"
causality=0
truncate --size 0 $dfile
begin=`shuf -i 0-1000 -n 1`
echo $begin > $dfile
echo "file initialised to $begin"
#VALGRIND="valgrind --track-origins=yes --log-file="valg""
for i in {1..4} 
do
	if [ $i -eq $tdaemon ]
	then
		$VALGRIND ../bin/node $i nodelist 1 `shuf -i 100-300 -n 1` $causality 100 $dfile > nocausalproc$i 2>&1 &
	else
		$VALGRIND ../bin/node $i nodelist 0 `shuf -i 100-300 -n 1` $causality 100 $dfile > nocausalproc$i 2>&1 &
	fi
done
