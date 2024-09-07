#!/bin/bash

sudo insmod fullfork.ko
for ((i=1; i<=50; i++))
do
	for((j=1; j<=100; j++))
	do
		sudo dmesg -c > /dev/null
    		./forker $i
    		time_info=$(dmesg | grep TIME_INFO)
   		echo "$time_info" >> "time_info_$i.txt"
		echo "$i - $j"
	done
done
sudo rmmod fullfork
