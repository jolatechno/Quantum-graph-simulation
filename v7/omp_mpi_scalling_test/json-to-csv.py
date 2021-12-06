#!/usr/bin/python3

import json
import functools
import os, sys

def prod(List):
	res = 1
	for x in List:
		res *= x
	return res
	
def string_to_key(str):
	keys = [int(i) for i in str.split(",")]
	if len(keys) == 2:
		keys.append(1)
	return (*keys,)

def compare(item1, item2):
	if prod(item1) != prod(item2):
		return prod(item2) - prod(item1)

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

	command = data["command"]
	command = command.split(" ")[1]

	n_iter = (command.split("|")[0]).split(",")[0]
	n_node = command.split("|")[1]
	rule = command.split("|")[2].replace(";", "_")

	string = "\"#n iters\",\"initial #n node\",\"rule\"\n"
	string += n_iter + "," + n_node + ",\"" + rule + "\"\n\n"  
	
	string += "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"#n object\",\"execution time\""
	for i in range(n_step):
		string += f",\"step {i}\""

	for i, key in enumerate(keys):
		string += "\n"

		n_thread, n_task, n_node = n_threads[i]
		this_step = data["results"][key]

		string += f"{ n_thread },{ n_task },{ n_node },{ this_step['num_object'] },{ this_step['total'] }"
		for i in range(n_step):
			string += f",{ this_step['steps'][i] }"
	
	write_file_name = "../../Quantum-graph-simulation-plots/scaling-plots/excel/" + rule + name_extension + ".csv"
	with open(write_file_name, "w") as f:
		f.write(string)
	