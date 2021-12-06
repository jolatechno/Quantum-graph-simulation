#!/usr/bin/python3

import json
import numpy as np
import functools
import os
import sys

def string_to_key(str):
	keys = [int(i) for i in str.split(",")]
	if len(keys) == 2:
		keys.append(1)
	return (*keys,)

def compare(item1, item2):
	if np.prod(item1) != np.prod(item2):
		return np.prod(item2) - np.prod(item2)

	if item1[2] != item2[2]:
		return item2[2] - item1[2]

	return item2[0] - item1[0]

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for Input in filenames:
	Input = Input.split(",")
	filename = Input[0]

	name_extension = ""
	if len(Input) > 1:
		name_extension = Input[1]

	with open(filename) as f:
		data = json.load(f)

	keys = list(data["results"].keys())
	n_threads = [string_to_key(key) for key in keys]
	n_threads.sort(key=functools.cmp_to_key(compare))

	n_step = len(data["results"][keys[0]]["steps"])

	rule = data["command"].split("|")[-1].replace(";", "_")

	string = "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"#n object\",\"execution time\""
	for i in range(n_step):
		string += f",\"step {i}\""
	string += "\n";

	for i, key in enumerate(keys):
		n_thread, n_task, n_node = n_threads[i]
		this_step = data["results"][key]

		string += f"{ n_thread },{ n_task },{ n_node },{ this_step['num_object'] },{ this_step['total'] }"
		for i in range(n_step):
			string += f",{ this_step['steps'][i] }"
		string += "\n"
	
	write_file_name = "../../Quantum-graph-simulation-plots/scaling-plots/excel/" + rule + name_extension + ".csv"
	with open(write_file_name, "w") as f:
		f.write(string)
	