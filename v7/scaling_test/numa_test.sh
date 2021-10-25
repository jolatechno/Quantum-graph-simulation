#!/bin/bash

print_usage() {
  printf "Usage: ./numa_test.sh ops...
	-h: this help menu
	-r: rule (default = 'split_merge')
	-s: initial state size (default = 12)
	-m: safety margin (default = 0.2)
	-d: delay befor recording numastat and killing the simulation (in second, default = 10s)
"
}

size=12
safety_margin=0.2
rule="split_merge"
delay=10

while getopts 'r:s:m:hd:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
	r) rule="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
	m) safety_margin="${OPTARG}" ;;
	d) delay="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

# command
command="OMP_PROC_BIND=true ./scaling_test.out 1000000,seed=0\|${size}\|${rule}"

echo "\"command\" : \"${command}\","
echo "\"results\" : "

# evaluate command and get pid
eval "${command} &> /dev/null &"
PID=$(echo $!)

# sleep
sleep ${delay}

# read numastat
numastat -p ${PID}

# kill command
kill ${PID}