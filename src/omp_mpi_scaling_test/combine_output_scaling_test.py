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


match = "out_" if len(sys.argv) == 1 else sys.argv[1]
print_match = "*" in match

def accumulate_steptime(indict):
	outdict = starting.copy()

	for key in indict.keys():
		value = indict[key]

		for outkey in translator[key]:
			outdict[outkey] += value

	return outdict

def num_node_from_file_name(filename):
	file_base = ".".join(filename.split(".")[:-1])
	num_node = file_base.split(match)[-1]

	return int(num_node)

def compare_filenames(filename1, filename2):
	num_node1, num_node2 = -1, -1

	try:
		num_node1 = num_node_from_file_name(filename1)
	except Exception as err:
		print(filename1, err, file=sys.stderr)

	try:
		num_node2 = num_node_from_file_name(filename2)
	except Exception as err:
		print(filename2, err, file=sys.stderr)

	return num_node1 - num_node2


output = {
	"command" : {
		"n_iter" : None,
		"n_node" : None,
		"rule": None
	},

	"num_nodes" : [],
	"num_jobs" : [],
	"num_threads" : [],

	"exec_time" : [],
	"total_num_objects" : [],
	"accuracy" : [],


	"proportions" : {
		key : [] for key in ordered_keys
	}
}

filenames = []
for dirpath, dirs, files in os.walk("tmp"): 
	filtered_files = fnmatch.filter(files, match + "*.out")
	filenames.extend([dirpath + "/" + filename for filename in filtered_files])

filenames.sort(key=functools.cmp_to_key(compare_filenames))


for filename in filenames:
	with open(filename, "r") as file:
		print(f"openning \"{ filename }\"", file=sys.stderr)
		try:
			num_node = num_node_from_file_name(filename)

			txt_file = file.read()
			json_file = "{" + "{".join(txt_file.split("{")[1:]).replace("nan", "0.0")

			json_dict = json.loads(json_file)




			if output["command"]["n_iter"] is None or output["command"]["n_node"] is None or output["command"]["rule"] is None:
				command = json_dict["command"]
				command = command.split(" ")[1]

				output["command"]["n_iter"] = (command.split("|")[0]).split(",")[0]
				output["command"]["n_node"] = command.split("|")[1]
				output["command"]["rule"]   = command.split("|")[2].replace(";", "_")


			for key in json_dict["results"].keys():
				key_split = key.split(",")
				num_threads, num_jobs = int(key_split[0]), int(key_split[1])

				output["num_nodes"].append(num_node)
				output["num_threads"].append(num_threads)
				output["num_jobs"].append(num_jobs)

				output["exec_time"].append(json_dict["results"][key]["total"])
				output["total_num_objects"].append(json_dict["results"][key]["num_object"])
				output["accuracy"].append(json_dict["results"][key]["total_proba"])

				avg_step_time = accumulate_steptime(json_dict["results"][key]["max_step_time"])
				for name in ordered_keys:
					output["proportions"][name].append(avg_step_time[name]/json_dict["results"][key]["total"])

		except Exception as err:
			print(filename, err, file=sys.stderr)


output_str = json.dumps(output, indent=4)
print(output_str)

isExist = os.path.exists("out")
if not isExist:
	os.makedirs("out")
with open("out/" + match + "combined.json.out", "w") as outfile:
	outfile.write(output_str)