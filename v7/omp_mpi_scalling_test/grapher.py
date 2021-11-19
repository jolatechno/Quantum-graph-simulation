#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import sys

width = 0.7
multi_threading = False

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]
if filenames == ["--mt"]:
	filenames.append("res.json")

for filename in filenames:
	if filename == "--mt":
		multi_threading = True
		continue

	with open(filename) as f:
		data = json.load(f)

	n_threads = list(data["results"].keys())[::-1]
	rule = data["command"].split("|")[-1].replace(";", "_")

	scaling = np.zeros((len(n_threads), len(data["results"]["1"]["steps"]) + 1))
	proportions = np.zeros((len(n_threads), len(data["results"]["1"]["steps"])))

	for i, n_thread in enumerate(n_threads):
		scaling[i, -1] = data["results"]["1"]["total"] / data["results"][n_thread]["total"]
		for step in range(proportions.shape[1]):
			proportions[i, step] = data["results"][n_thread]["steps"][step] / data["results"][n_thread]["total"]
			scaling[i, step] = data["results"]["1"]["steps"][step] / data["results"][n_thread]["steps"][step]





	# bar positions
	bar_width = width / (scaling.shape[1] - 1)
	tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
	bar_starting_position = tick_position - bar_width * (scaling.shape[1] - 1) / 2


	# second bar positions
	bar_width_1 = width / (proportions.shape[1] - 1)
	tick_position_1 = np.arange(len(n_threads), dtype=float)  # the label locations
	bar_starting_position_1 = tick_position - bar_width_1 * (proportions.shape[1] - 1) / 2


	# limit graph values
	x_points, y_points = [[]], [[]]
	total_end = bar_starting_position[-1] + bar_width*proportions.shape[1] + bar_width/2
	total_begin = bar_starting_position[0] - bar_width/2
	for i, n_thread in enumerate(n_threads):
		begin = bar_starting_position[i] - bar_width/2
		end = bar_starting_position[i] + bar_width*proportions.shape[1] + bar_width/2

		n_thread = int(n_thread) if not multi_threading or i < len(n_threads) - 1 else int(n_threads[-2])
		if not multi_threading or i < len(n_threads) - 2:
			x_points.append([end, total_end])
			y_points.append([n_thread, n_thread])

		x_points[0].append(begin)
		y_points[0].append(n_thread)

		x_points[0].append(end)
		y_points[0].append(n_thread)





	# plot wall time scalling
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("execution time scaling for each step")
	ax.set_xticks(tick_position)
	ax.set_xticklabels(n_threads)

	for step in range(proportions.shape[1]):
		ax.bar(bar_starting_position + bar_width*step, scaling[:, step], width=bar_width, label=f'step { step + 1 }')

	ax.bar(bar_starting_position + bar_width*proportions.shape[1], scaling[:, -1], width=bar_width, label='total')

	ax.plot(x_points[0], y_points[0], "k--")
	for i in range(1, len(x_points)):
		ax.plot(x_points[i], y_points[i], "--", color="dimgrey")
	ax.set_yscale('log')

	# saving fig
	ax.legend(loc='upper left')
	fig.savefig("plots/scaling/scaling_" + rule + ".png")





	# plot proportions
	fig, ax = plt.subplots(1, 1, figsize=(10, 5), constrained_layout=True)
	ax.set_title("proportion of the total execution time taken by each step")
	ax.set_xticks(tick_position_1)
	ax.set_xticklabels(n_threads)

	for step in range(proportions.shape[1]):
		ax.bar(bar_starting_position_1 + bar_width_1*step, proportions[:, step], width=bar_width_1, label=f'step { step + 1 }')

	# saving fig
	ax.legend(loc='upper left')
	fig.savefig("plots/proportions/proportions_" + rule + ".png")