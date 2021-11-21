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

	for i, n_thread in enumerate(n_threads):
		scaling[i, -1] = data["results"][n_threads[0]]["total"] / data["results"][n_thread]["total"]
		for step in range(proportions.shape[1]):
			proportions[i, step] = data["results"][n_thread]["steps"][step] / data["results"][n_thread]["total"]
			scaling[i, step] = data["results"][n_threads[0]]["steps"][step] / data["results"][n_thread]["steps"][step]




	# limit graph values
	x_points, y_points = [[]], [[]]
	for i, n_thread in enumerate(n_threads):
		n_thread = np.product([int(x) for x in n_thread.split(',')])
		if i < len(n_threads) - 2:
			n_thread_1 = np.product([int(x) for x in n_threads[i + 1].split(',')])
			if n_thread_1 > n_thread:
				x_points.append([i + 0.2, len(n_threads) - 1])
				y_points.append([n_thread, n_thread])

		x_points[0].append(i - 0.2)
		y_points[0].append(n_thread)

		x_points[0].append(i + 0.2)
		y_points[0].append(n_thread)





	# plot wall time scalling
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("execution time scaling for each step")
	ax.set_xticks(np.arange(0, len(n_threads)))
	ax.set_xticklabels(n_threads)

	for step in range(proportions.shape[1]):
		ax.plot(scaling[:, step], label=f'step { step + 1 }')

	ax.plot(scaling[:, -1], linewidth=4, label='total')

	ax.plot(x_points[0], y_points[0], "k--", label='total number of threads')
	for i in range(1, len(x_points)):
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