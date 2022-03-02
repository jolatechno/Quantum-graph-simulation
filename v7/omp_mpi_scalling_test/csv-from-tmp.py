#!/usr/bin/python

import os, sys, fnmatch
import functools
import json

ordered_keys = ["pre-symbolic", "symbolic_iteration", "collisions", "pre-final", "final_iteration", "communication"]

starting = {
	"symbolic_iteration" : 0.0,
	"pre-symbolic" : 0.0,
	"collisions" : 0.0,
	"pre-final" : 0.0,
	"final_iteration" : 0.0,
	"communication" : 0.0
}

translator = {
	"num_child" : ["pre-symbolic"], 
	"prepare_index" : ["pre-symbolic"], 
	"equalize_child" : ["communication", "pre-symbolic"], 
	"truncate_symbolic" : ["pre-symbolic"],

	"symbolic_iteration" : ["symbolic_iteration"], 

	"compute_collisions - com" : ["communication", "collisions"], 
    "compute_collisions - finalize" : ["collisions"], 
    "compute_collisions - insert" : ["collisions"], 
    "compute_collisions - prepare" : ["collisions"], 

    "truncate" : ["pre-final"], 
    "prepare_final" : ["pre-final"], 

    "final" : ["final_iteration"], 
    "normalize" : ["final_iteration"], 
    "equalize" : ["communication", "final_iteration"]
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


out_dict = {"results" : {}}

match = "out_" if len(sys.argv) == 1 else sys.argv[1]

for dirpath, dirs, files in os.walk("tmp"): 
  	for filename in fnmatch.filter(files, match + "*.out"):
  		with open("tmp/" + filename, "r") as file:
  			num_node = int(filename.split(".")[0].split(match)[1])

  			try:
	  			txt_file = file.read()
	  			json_file = "{" + "{".join(txt_file.split("{")[1:]).replace("nan", "0.0")

	  			json_dict = json.loads(json_file)
	  			out_dict["command"] = json_dict["command"]

	  			for key in json_dict["results"].keys():
	  				name = key + "," + str(num_node)
	  				out_dict["results"][name] = json_dict["results"][key]
	  		except Exception as err:
	  			print(filename, err)

keys = list(out_dict["results"].keys())

n_threads = [string_to_key(key) for key in keys]
n_threads.sort(key=functools.cmp_to_key(compare))

keys = [",".join([str(x) for x in key]) for key in n_threads]

command = out_dict["command"]
command = command.split(" ")[1]

n_iter = (command.split("|")[0]).split(",")[0]
n_node = command.split("|")[1]
rule = command.split("|")[2].replace(";", "_")

string = "\"#n iters\",\"initial #n node\",\"rule\"\n"
string += n_iter + "," + n_node + ",\"" + rule + "\"\n\n" 

string += "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"#n object\",\"execution time\",\""
string += "\",\"".join(ordered_keys) + "\""

for i, key in enumerate(keys):
	string += "\n";

	n_thread, n_task, n_node = n_threads[i]
	this_step = out_dict["results"][key]

	string += str(n_thread) + "," + str(n_task) + "," + str(n_node) + "," + str(this_step["num_object"]) + "," + str(this_step["total"])

	avg_step_time = accumulate_steptime(this_step["avg_step_time"])
	for name in ordered_keys:
		string += "," + str(avg_step_time[name])

print(string)
