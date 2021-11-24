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

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]
if filenames == ["--mt"]:
	filenames.append("res.json")

for filename in filenames:
	name_extension = ""
	if len(filename.split(",")) == 2:
		filename, name_extension = filename.split(",")

	with open(filename) as f:
		data = json.load(f)

	n_threads = list(data["results"].keys())
	n_threads.sort(key=functools.cmp_to_key(compare))
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




	# limit graph values
	x_points, y_points = [[], [], []], [[], [], []]
	min_num_threads = num_threads[0]
	for i in range(0, len(n_threads)):
		_, n_nodes = n_threads[i].split(",")
		max_scaling = num_threads[i]/min_num_threads

		if i < len(n_threads) - 2:
			max_scaling_1 = num_threads[i + 1]/min_num_threads
			if max_scaling_1 > max_scaling:
				x_points.append([i + 0.2, len(n_threads) - 1])
				y_points.append([max_scaling, max_scaling])

		x_points[0].append(i - 0.2)
		y_points[0].append(max_scaling)
		x_points[0].append(i + 0.2)
		y_points[0].append(max_scaling)

		x_points[1].append(i - 0.2)
		y_points[1].append(n_nodes)
		x_points[1].append(i + 0.2)
		y_points[1].append(n_nodes)





	# plot wall time scalling
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("execution time scaling for each step")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels(num_threads)

	for step in range(proportions.shape[1]):
		ax.plot(scaling[:, step], label=f'step { step + 1 }')

	ax.plot(scaling[:, -1], linewidth=4, label='total')

	max_scalling = np.amax(scaling)
	if max_scalling > np.sqrt(y_points[0][-1]):
		ax.plot(x_points[0], y_points[0], "k--", label='total number of thread')
		ax.plot(x_points[1], y_points[1], "r--", label='number of mpi ranks')
		for i in range(3, len(x_points)):
			ax.plot(x_points[i], y_points[i], "--", color="dimgrey")
	ax.set_yscale('log')

	ymin, ymax = ax.get_ylim()
	ax.set_ylim(max(ymin, 0.2), ymax)

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	ax.set_xlim(0, len(n_threads) - 1)
	fig.savefig("plots/scaling/scaling_" + rule + name_extension + ".png")





	# plot proportions
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("proportion of the total execution time taken by each step")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels([np.product([int(x) for x in n_thread.split(',')]) for n_thread in n_threads])

	for step in range(proportions.shape[1]):
		ax.plot(proportions[:, step], label=f'step { step + 1 }')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	ax.set_xlim(0, len(n_threads) - 1)
	fig.savefig("plots/proportions/proportions_" + rule + name_extension + ".png")





	# plot properties
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax2 = ax.twinx()
	ax.set_title("number of object per thread")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels([np.product([int(x) for x in n_thread.split(',')]) for n_thread in n_threads])

	ax.plot(object_per_threads, "b-", label='number of object per thread')
	ax2.plot(num_object, "r-", label='total number of object')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	ax.set_xlim(0, len(n_threads) - 1)
	ymin, ymax = ax.get_ylim()
	ax.set_ylim(0, ymax)
	fig.savefig("plots/properties/properties_" + rule + name_extension + ".png")