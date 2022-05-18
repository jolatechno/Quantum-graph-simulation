#!/usr/bin/python3

import os, sys, fnmatch
import functools
import json

ordered_keys = ["pre-symbolic-iteration", "symbolic-iteration", "collisions", "pre-object-generation", "object-generation", "communication"]

starting = {
	"symbolic-iteration" : 0.0,
	"pre-symbolic-iteration" : 0.0,
	"collisions" : 0.0,
	"pre-object-generation" : 0.0,
	"object-generation" : 0.0,
	"communication" : 0.0
}

# set to false to exlude comm from the rest of the dissection
sum_equal_one = True

translator = {
	"num_child" : ["pre-symbolic-iteration"], 
	"prepare_index" : ["pre-symbolic-iteration"], 
	"equalize_child" : ["communication"] if sum_equal_one else ["communication", "pre-symbolic-iteration"],
	"equalize_object" : ["communication"] if sum_equal_one else ["communication", "pre-symbolic-iteration"],
	"truncate_symbolic - prepare" : ["pre-symbolic-iteration"],
	"truncate_symbolic" : ["pre-symbolic-iteration"],

	"symbolic_iteration" : ["symbolic-iteration"], 

	"compute_collisions - com" : ["communication"] if sum_equal_one else ["communication", "collisions"],
    "compute_collisions - finalize" : ["collisions"], 
    "compute_collisions - insert" : ["collisions"], 
    "compute_collisions - prepare" : ["collisions"], 

    "truncate - prepare" : ["pre-symbolic-iteration"],
    "truncate" : ["pre-object-generation"], 
    "prepare_final" : ["pre-object-generation"], 

    "final" : ["object-generation"], 
    "normalize" : ["object-generation"],
}

def accumulate_steptime(indict):
	outdict = starting.copy()

	for key in indict.keys():
		value = indict[key]

		for outkey in translator[key]:
			outdict[outkey] += value

	return outdict

def prod(List):
	res = 1
	for x in List:
		res *= x
	return res

def string_to_key(str):
	keys = [int(i) for i in str.split(",")]
	if len(keys) == 2:
		keys.append(1)
	return keys

def compare(item1, item2):
	if prod(item1) != prod(item2):
		return prod(item2) - prod(item1)

	if item1[2] != item2[2]:
		return item2[2] - item1[2]

	return item2[0] - item1[0]


results = {}
command = "no_file_provided 0|0|"

match = "out_" if len(sys.argv) == 1 else sys.argv[1]

for dirpath, dirs, files in os.walk("tmp"): 
  	for filename in fnmatch.filter(files, match + "*.out"):
  		with open("tmp/" + filename, "r") as file:
  			num_node = int(filename.split(".")[0].split(match)[1])

  			try:
	  			txt_file = file.read()
	  			json_file = "{" + "{".join(txt_file.split("{")[1:]).replace("nan", "0.0")

	  			json_dict = json.loads(json_file)
	  			command = json_dict["command"]

	  			for key in json_dict["results"].keys():
	  				name = key + "," + str(num_node)
	  				results[name] = json_dict["results"][key]

	  		except Exception as err:
	  			print(filename, err)

keys = list(results.keys())

n_threads = [string_to_key(key) for key in keys]
n_threads.sort(key=functools.cmp_to_key(compare))

keys = [",".join([str(x) for x in key]) for key in n_threads]

command = command.split(" ")[1]

n_iter = (command.split("|")[0]).split(",")[0]
n_node = command.split("|")[1]
rule = command.split("|")[2].replace(";", "_")

print("\"#n iters\",\"initial #n node\",\"rule\"")
print(n_iter + "," + n_node + ",\"" + rule + "\"\n")

string = "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"\",\"#n object\",\"#n symb. object\",\"total_proba\",\"\",\"execution time\",\""
string += "\",\"".join(ordered_keys) + "\""
string += ",\"\",\"time inbalance\",\"NoO inbalance\",\"NoO symb. inbalance\""
print(string)

for i, key in enumerate(keys):
	string = "";

	n_thread, n_task, n_node = n_threads[i]
	total_n_task = n_task*n_node
	this_step = results[key]

	NoO_inbalance = (this_step['max_num_object'] - this_step['avg_num_object'])/this_step['avg_num_object']
	NoO_symb_inbalance = (this_step['max_symbolic_num_object'] - this_step['avg_symbolic_num_object'])/this_step['avg_symbolic_num_object']

	string += f"{n_thread},{n_task},{n_node},,{this_step['num_object']},{this_step['avg_symbolic_num_object']*total_n_task},{this_step['total_proba']:e},,{this_step['total']}"

	avg_step_time = accumulate_steptime(this_step["max_step_time"])
	for name in ordered_keys:
		string += "," + str(avg_step_time[name])

	string += f",,{this_step['total_relative_inbalance']},{NoO_inbalance},{NoO_symb_inbalance}"
	
	print(string)