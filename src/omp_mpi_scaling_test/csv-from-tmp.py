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

results = {}
command = "no_file_provided 0|0|"

match = "out_" if len(sys.argv) == 1 else sys.argv[1]
print_match = "*" in match

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
	if not print_match:
		keys = [""] + [int(i) for i in str.split(",")]
		if len(keys) == 2:
			keys.append(1)
		return keys
	else:
		keys = [str.split(",")[0]] + [int(i) for i in str.split(",")[1:]]
		if len(keys) == 3:
			keys.append(1)
		return keys

def key_to_string(key):
	return ",".join(['"' + key[0] + '"'] + [str(i) for i in key[1:]])

def compare(item1, item2):
	if item1[0] < item2[0]:
		return 1
	if item1[0] > item2[0]:
		return -1

	if prod(item1[1:]) != prod(item2[1:]):
		return prod(item2[1:]) - prod(item1[1:])

	if item1[3] != item2[3]:
		return item2[3] - item1[3]

	return item2[1] - item1[1]

def get_match(str):
	begin = match.split("*")[0]
	end = match.split("*")[-1]

	match_ = str[1:-1]
	match_ = begin.join(match_.split(begin)[1:])
	match_ = end.join(match_.split(end)[:-1])

	return match_

for dirpath, dirs, files in os.walk("tmp"): 
  	for filename in fnmatch.filter(files, match + "*.out"):
  		with open("tmp/" + filename, "r") as file:
  			print(f"openning \"{ filename }\"", file=sys.stderr)
  			try:
  				file_base = filename.split(".")[0]
  				num_node = file_base.split(match[-1])[-1]

  				# brake if not a number
  				if not print_match:
  					if file_base.split(match)[1][0] not in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']:
	  					print(f"\tnot taking \"{filename}\" into account", file=sys.stderr)
	  					continue

	  				file_base = ""

  				file_base = file_base[:-len(num_node)]
  				num_node = int(num_node)






	  			txt_file = file.read()
	  			json_file = "{" + "{".join(txt_file.split("{")[1:]).replace("nan", "0.0")

	  			json_dict = json.loads(json_file)
	  			command = json_dict["command"]

	  			for key in json_dict["results"].keys():
	  				name = '"' + file_base + '",' + key + "," + str(num_node)
	  				results[name] = json_dict["results"][key]

	  		except Exception as err:
	  			print(filename, err, file=sys.stderr)

keys = list(results.keys())

n_threads = [string_to_key(key) for key in keys]
n_threads.sort(key=functools.cmp_to_key(compare))

keys = [key_to_string(key) for key in n_threads]

command = command.split(" ")[1]

n_iter = (command.split("|")[0]).split(",")[0]
n_node = command.split("|")[1]
rule = command.split("|")[2].replace(";", "_")

print("", file=sys.stderr)
print("\"#n iters\",\"initial #n node\",\"rule\"")
print(n_iter + "," + n_node + ",\"" + rule + "\"\n")

string = ""
if print_match:
	string += "\"file_base\","
string += "\"#n thread per rank\",\"#n task per node\",\"#n node\",\"\",\"#n object\",\"#n symb. object\",\"total_proba\",\"\",\"execution time\",\""
string += "\",\"".join(ordered_keys) + "\""
string += ",\"\",\"time inbalance\",\"NoO inbalance\",\"NoO symb. inbalance\""
print(string)

for i, key in enumerate(keys):
	string = "";

	base_name, n_thread, n_task, n_node = n_threads[i]
	total_n_task = n_task*n_node
	this_step = results[key]

	NoO_inbalance = (this_step['max_num_object'] - this_step['avg_num_object'])/this_step['avg_num_object']
	NoO_symb_inbalance = (this_step['max_symbolic_num_object'] - this_step['avg_symbolic_num_object'])/this_step['avg_symbolic_num_object']

	if print_match:
		string += f"\"{get_match(base_name)}\",{n_thread},{n_task},{n_node},,{this_step['num_object']},{this_step['avg_symbolic_num_object']*total_n_task},{this_step['total_proba']:e},,{this_step['total']}"
	else:
		string += f"{n_thread},{n_task},{n_node},,{this_step['num_object']},{this_step['avg_symbolic_num_object']*total_n_task},{this_step['total_proba']:e},,{this_step['total']}"

	avg_step_time = accumulate_steptime(this_step["max_step_time"])
	for name in ordered_keys:
		string += "," + str(avg_step_time[name])

	string += f",,{this_step['total_relative_inbalance']},{NoO_inbalance},{NoO_symb_inbalance}"
	
	print(string)