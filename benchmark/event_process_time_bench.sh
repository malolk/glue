#!/bin/bash

WRITE_NUM=100000
EXEC_PATH=../bin/TestEventProcessTime
for client_num in 100 1000
do
	for pipe_num in 1000 5000 10000 15000 20000
	do 
		echo client_num=${client_num}, pipe_num=${pipe_num}
		${EXEC_PATH} -n ${pipe_num} -a ${client_num} -w ${WRITE_NUM}
	done
done

