#!/usr/bin/python3

import os, sys, fnmatch
import json

out_dict = {"results" : {}}

def multiply_key(key, num_node):
	n_thread, n_nodes = [int(x) for x in key.split(',')]
	n_nodes *= num_node
	return str(n_thread) + "," + str(n_nodes)

matches = ["out_"] if len(sys.argv) == 1 else sys.argv[1:]
for match in matches:
	for dirpath, dirs, files in os.walk('tmp'): 
  		for filename in fnmatch.filter(files, match + '*.out'):
  			with open("tmp/" + filename, "r") as file:
  				num_node = int(filename.split(".")[0].split(match)[1])

  				txt_file = file.read()
  				json_file = "{" + "{".join(txt_file.split("{")[1:])

  				json_dict = json.loads(json_file)
  				out_dict["command"] = json_dict["command"]

  				for key in json_dict["results"].keys():
  					out_dict["results"][multiply_key(key, num_node)] = json_dict["results"][key]

	print(json.dumps(out_dict, indent=4))
			


