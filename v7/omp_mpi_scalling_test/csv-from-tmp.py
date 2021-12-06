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

n_step = len(out_dict["results"][keys[0]]["steps"])

string = "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"#n object\",\"execution time\""
for i in range(n_step):
	string += ",\"step " + str(i) + "\""
string += "\n";

for i, key in enumerate(keys):
	n_thread, n_task, n_node = n_threads[i]
	this_step = out_dict["results"][key]

	string += str(n_thread) + "," + str(n_task) + "," + str(n_node) + "," + str(this_step["num_object"]) + "," + str(this_step["total"])
	for i in range(n_step):
		string += "," + str(this_step["steps"][i])
	string += "\n"

print(string)
