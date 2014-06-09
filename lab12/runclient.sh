#!/bin/bash
# Program:
#       The script runs client for 20 times, kill all after 30 secs
# History:
#       2014/06/09  Si-Hao Wu   FIrst release

# ./client 192.168.92.231 12121
# killall gnome-terminal

for ((a = 0; a < 20; a++))
do
    gnome-terminal -e "bash -c \"./client 192.168.92.231 12121; exec bash\""
done
