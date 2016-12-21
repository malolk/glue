#!/bin/bash

running_time=60
thread_num=2
for client_num in 10 20 30 40 50 100 1000 5000 10000 15000 20000
do
	for chunk_size in 16 48
	do
		echo RunningTime=${running_time}s, client_num=${client_num}, \
		chunk_size=${chunk_size}, thread_num=${thread_num}
		../bin/throughput_client_test -c ${client_num} \
		-k ${chunk_size} -t ${running_time} -T ${thread_num} 
		sleep 5
	done
done


