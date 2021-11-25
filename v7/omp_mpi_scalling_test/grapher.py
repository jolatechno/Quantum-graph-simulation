#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import functools
import sys

def compare(item1, item2):
	n_thread_1, n_nodes_1 = [int(x) for x in item1.split(',')]
	n_thread_2, n_nodes_2 = [int(x) for x in item2.split(',')]

	if n_thread_1*n_nodes_1 != n_thread_2*n_nodes_2:
		return n_thread_1*n_nodes_1 - n_thread_2*n_nodes_2

	return n_thread_2 - n_thread_1

def clear_tick_label(ax):
	threshold = 30
	for t in ax.get_xticklabels():
	    if len(t.get_text()) > threshold:
	        t.set_rotation(90)

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]
if filenames == ["--mt"]:
	filenames.append("res.json")

for Input in filenames:
	Input = Input.split(",")
	filename = Input[0]

	no_x_label, only_omp, only_mpi, name_extension = False, False, False, ""
	if len(Input) > 1:
		name_extension = Input[1]
	for i in range(2, len(Input)):
		no_x_label = no_x_label | (Input[i] == "no_x_label")
		only_mpi = only_mpi | (Input[i] == "only_mpi")
		only_omp = only_omp | (Input[i] == "only_omp")

	with open(filename) as f:
		data = json.load(f)

	n_threads = list(data["results"].keys())
	n_threads.sort(key=functools.cmp_to_key(compare))
	print(only_mpi, only_omp, n_threads)
	if only_mpi:
		n_threads = list(filter(lambda threads : int(threads.split(",")[1]) > 1, n_threads))
	if only_omp:
		n_threads = list(filter(lambda threads : int(threads.split(",")[1]) == 1, n_threads))
	print(only_mpi, only_omp, n_threads)
	print()
	num_threads = [np.product([int(x) for x in n_thread.split(',')]) for n_thread in n_threads]

	rule = data["command"].split("|")[-1].replace(";", "_")

	scaling = np.zeros((len(n_threads), len(data["results"][n_threads[0]]["steps"]) + 1))
	proportions = np.zeros((len(n_threads), len(data["results"][n_threads[0]]["steps"])))
	num_object = np.zeros(len(n_threads))
	object_per_threads = np.zeros(len(n_threads))

	for i, n_thread in enumerate(n_threads):
		total = data["results"][n_thread]["total"]

		scaling[i, -1] = 1 if total == 0 else data["results"][n_threads[0]]["total"] / total
		for step in range(proportions.shape[1]):
			base = data["results"][n_thread]["steps"][step]

			proportions[i, step] = 1 if total == 0 else data["results"][n_thread]["steps"][step] / total
			scaling[i, step] = 1 if base == 0 else data["results"][n_threads[0]]["steps"][step] / base

			total_n_thread = np.product([int(x) for x in n_thread.split(',')])
			num_object[i] = data["results"][n_thread]["num_object"]
			object_per_threads[i] = num_object[i] / total_n_thread




	# intialize x values
	offset = 1
	log_xlist = num_threads.copy()
	for i in range(1, len(num_threads)):
		if num_threads[i] == num_threads[i - 1]:
			offset *= 1.1
		log_xlist[i] *= offset

	# find the first x of the slope
	first_idx = 0
	while first_idx < len(num_threads) - 1:
		scaling_ratio = scaling[first_idx + 1, -1] / scaling[first_idx, -1]
		num_thread_ratio = num_threads[first_idx + 1] / num_threads[first_idx]

		if scaling_ratio > num_thread_ratio**0.7 and num_thread_ratio > 1:
			break

		first_idx += 1
		if first_idx == len(num_threads) - 1:
			first_idx += 1

	# create perfect scaling
	perfect_scaling, x_perfect_scalling = [], []
	for i in range(first_idx, len(num_threads)):
		x_perfect_scalling.append(log_xlist[i])
		perfect_scaling.append(num_threads[i] / num_threads[first_idx])

	# intialize labels
	labels = [num_threads[0]]
	for i in range(1, len(num_threads)):
		if num_threads[i] != num_threads[i - 1]:
			labels.append(str(num_threads[i]))
			if i < len(num_threads) - 1 and num_threads[i] == num_threads[i + 1]:
				labels[-1] += "..."
		else:
			labels.append("")


	# limit graph values
	min_num_threads = num_threads[0]





	# plot wall time scalling
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("execution time scaling for each step")

	ax.set_yscale('log')
	ax.set_xscale('log')
	ax.get_xaxis().set_tick_params(which='minor', size=0)
	ax.get_xaxis().set_tick_params(which='minor', width=0) 

	if not no_x_label:
		ax.set_xticks(log_xlist)
		ax.set_xticklabels(labels)

	for step in range(proportions.shape[1]):
		ax.plot(log_xlist, scaling[:, step], label=f'step { step + 1 }')

	ax.plot(log_xlist, scaling[:, -1], linewidth=4, label='total')

	if len(perfect_scaling) > 0:
		ax.plot(x_perfect_scalling, perfect_scaling, "k--", label='ideal scaling')

	ymin, ymax = ax.get_ylim()
	ax.set_ylim(max(ymin, 0.2), ymax**1.1)

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	fig.savefig("plots/scaling/scaling_" + rule + name_extension + ".png")





	# plot proportions
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("proportion of the total execution time taken by each step")

	ax.set_xscale('log')
	ax.get_xaxis().set_tick_params(which='minor', size=0)
	ax.get_xaxis().set_tick_params(which='minor', width=0) 

	if not no_x_label:
		ax.set_xticks(log_xlist)
		ax.set_xticklabels(labels)

	for step in range(proportions.shape[1]):
		ax.plot(log_xlist, proportions[:, step], label=f'step { step + 1 }')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	fig.savefig("plots/proportions/proportions_" + rule + name_extension + ".png")





	# plot properties
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("number of object")

	ax.set_yscale('log')
	ax.set_xscale('log')
	ax.get_xaxis().set_tick_params(which='minor', size=0)
	ax.get_xaxis().set_tick_params(which='minor', width=0) 

	if not no_x_label:
		ax.set_xticks(log_xlist)
		ax.set_xticklabels(labels)

	ax2 = ax.twinx()
	ax2.set_yscale('log')

	ax.plot(log_xlist, object_per_threads, "b-", label='number of object per thread')
	ax2.plot(log_xlist, num_object, "r-", label='total number of object')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1.05, 0.45))
	ax2.legend(loc='center left', bbox_to_anchor=(1.05, 0.55))
	fig.savefig("plots/properties/properties_" + rule + name_extension + ".png")