#!/bin/bash

print_usage() {
  printf "Usage: ./numa_test.sh ops...
	-h: this help menu
	-r: rule (default = 'split_merge')
	-s: initial state size (default = 12)
	-m: safety margin (default = 0.2)
	-a: additional arguments
	-d: delay befor recording numastat and killing the simulation (in second, default = 10s)
"
}

size=12
safety_margin=0.2
rule="split_merge"
delay=10
args=""

while getopts 'r:s:m:a:hd:' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
	r) rule="${OPTARG}" ;;
    s) size="${OPTARG}" ;;
	m) safety_margin="${OPTARG}" ;;
	a) args="${OPTARG}" ;;
	d) delay="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done

# command
command="./state_test.out -n 1000000 -v 1 -r ${rule} -s ${size} --safety-margin ${safety_margin} --seed 0 ${args}"

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