#!/usr/bin/python3

import os, sys
import json

filename = "out_1.out" if len(sys.argv) == 1 else sys.argv[1]

with open("tmp/" + filename, "r") as file:
	txt_file = file.read()
	json_file = "{" + "{".join(txt_file.split("{")[1:]).replace("nan", "0.0")

	json_dict = json.loads(json_file)
	command = json_dict["command"]
	
	key = list(json_dict["results"].keys())[0]
	min_memory_usage = json_dict["results"][key]["min_memory_usage"]
	max_memory_usage = json_dict["results"][key]["max_memory_usage"]
	avg_memory_usage = json_dict["results"][key]["avg_memory_usage"]

	step_accuracy = json_dict["results"][key]["step_accuracy"]

command = command.split(" ")[1]

n_iter = (command.split("|")[0]).split(",")[0]
n_node = command.split("|")[1]
rule = command.split("|")[2].replace(";", "_")

print("\"#n iters\",\"initial #n node\",\"rule\"")
print(n_iter + "," + n_node + ",\"" + rule + "\"\n")

print("\"step_accuracy\",,\"min_memory_usage\",\"max_memory_usage\",\"avg_memory_usage\"")

for acc, Min, Max, Avg in zip(step_accuracy, min_memory_usage, max_memory_usage, avg_memory_usage):
	print(f"{acc},,{Min},{Max},{Avg}")