#!/usr/bin/python

import os, sys, fnmatch
import functools
import json

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
	  			json_file = "{" + "{".join(txt_file.split("{")[1:])

	  			json_dict = json.loads(json_file)
	  			out_dict["command"] = json_dict["command"]

	  			for key in json_dict["results"].keys():
	  				out_dict["results"][key + "," + str(num_node)] = json_dict["results"][key]
	  		except Exception:
	  			pass

keys = list(out_dict["results"].keys())

n_threads = [string_to_key(key) for key in keys]
n_threads.sort(key=functools.cmp_to_key(compare))

keys = [",".join([str(x) for x in key]) for key in n_threads]

n_step = len(out_dict["results"][keys[0]]["max_step_time"])
command = out_dict["command"]
command = command.split(" ")[1]

n_iter = (command.split("|")[0]).split(",")[0]
n_node = command.split("|")[1]
rule = command.split("|")[2].replace(";", "_")

string = "\"#n iters\",\"initial #n node\",\"rule\"\n"
string += n_iter + "," + n_node + ",\"" + rule + "\"\n\n" 

string += "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"#n object\",\"execution time\""
for i in range(n_step):
	string += ",\"max step " + str(i) + "\""
string += ","
for i in range(n_step):
	string += ",\"min step " + str(i) + "\""
string += ","
for i in range(n_step):
	string += ",\"cpu step " + str(i) + "\""

for i, key in enumerate(keys):
	string += "\n";

	n_thread, n_task, n_node = n_threads[i]
	this_step = out_dict["results"][key]

	string += str(n_thread) + "," + str(n_task) + "," + str(n_node) + "," + str(this_step["num_object"]) + "," + str(this_step["total"])
	for i in range(n_step):
		string += "," + str(this_step["max_step_time"][i])
	string += ","
	for i in range(n_step):
		string += "," + str(this_step["min_step_time"][i])
	string += ","
	for i in range(n_step):
		string += "," + str(this_step["avg_cpu_step_time"][i])

print(string)
