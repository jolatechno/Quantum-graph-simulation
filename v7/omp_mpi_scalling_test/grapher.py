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
	with open(filename) as f:
		data = json.load(f)

	n_threads = list(data["results"].keys())
	n_threads.sort(key=functools.cmp_to_key(compare))
	rule = data["command"].split("|")[-1].replace(";", "_")

	scaling = np.zeros((len(n_threads), len(data["results"][n_threads[0]]["steps"]) + 1))
	proportions = np.zeros((len(n_threads), len(data["results"][n_threads[0]]["steps"])))
	object_per_threads = np.zeros(len(n_threads))

	for i, n_thread in enumerate(n_threads):
		scaling[i, -1] = data["results"][n_threads[0]]["total"] / data["results"][n_thread]["total"]
		for step in range(proportions.shape[1]):
			proportions[i, step] = data["results"][n_thread]["steps"][step] / data["results"][n_thread]["total"]
			scaling[i, step] = data["results"][n_threads[0]]["steps"][step] / data["results"][n_thread]["steps"][step]

			total_n_thread = np.product([int(x) for x in n_thread[i].split(',')])
			object_per_threads[i] = data["results"][n_thread]["num_object"] / total_n_thread




	# limit graph values
	x_points, y_points = [[], [], []], [[], [], []]
	for i in range(0, len(n_threads)):
		n_thread, n_nodes = [int(x) for x in n_threads[i].split(',')]
		total_n_thread = n_thread*n_nodes

		if i < len(n_threads) - 2:
			total_n_thread_1 = np.product([int(x) for x in n_threads[i + 1].split(',')])
			if total_n_thread_1 > total_n_thread:
				x_points.append([i + 0.2, len(n_threads) - 1])
				y_points.append([total_n_thread, total_n_thread])

		x_points[0].append(i - 0.2)
		y_points[0].append(total_n_thread)
		x_points[0].append(i + 0.2)
		y_points[0].append(total_n_thread)

		x_points[1].append(i - 0.2)
		y_points[1].append(n_nodes)
		x_points[1].append(i + 0.2)
		y_points[1].append(n_nodes)





	# plot wall time scalling
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("execution time scaling for each step")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels([np.product([int(x) for x in n_thread.split(',')]) for n_thread in n_threads])

	for step in range(proportions.shape[1]):
		ax.plot(scaling[:, step], label=f'step { step + 1 }')

	ax.plot(scaling[:, -1], linewidth=4, label='total')

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
	fig.savefig("plots/scaling/scaling_" + rule + ".png")





	# plot proportions
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("proportion of the total execution time taken by each step")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels(n_threads)

	for step in range(proportions.shape[1]):
		ax.plot(proportions[:, step], label=f'step { step + 1 }')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	ax.set_xlim(0, len(n_threads) - 1)
	fig.savefig("plots/proportions/proportions_" + rule + ".png")





	# plot properties
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("number of object per thread")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels(n_threads)

	ax.plot(object_per_threads, label=f'number of object per thread')

	# saving fig
	ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	ax.set_xlim(0, len(n_threads) - 1)
	fig.savefig("plots/properties/properties_" + rule + ".png")