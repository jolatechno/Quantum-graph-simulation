#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
import numpy as np
import os
import sys

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for filename in filenames:
	with open(filename) as f:
	  data = json.load(f)

	# find name
	for i in range(1000000):
		name = f"{ i }_{ data['rule'] }.png"
		if not os.path.exists("plots/stats/" + name):
			break

	n_iterations = len(data["iterations"])
	iterations_list = np.arange(0, n_iterations)

	total_probas = np.array([it["total_proba"] for it in data["iterations"]])
	ratios = np.array([it["ratio"] for it in data["iterations"]])
	num_graphs = np.array([it["num_graphs"] for it in data["iterations"]])

	avg_size = np.array([it["avg_size"] for it in data["iterations"]])
	std_dev_size = np.array([it["std_dev_size"] for it in data["iterations"]])

	avg_density = np.array([it["avg_density"] for it in data["iterations"]])
	std_dev_density = np.array([it["std_dev_density"] for it in data["iterations"]])

	# probability
	fig = plt.figure()
	ax = plt.axes()
	ax.set_xlabel('iterations')
	ax.set_title(f'total probabilty and number of graph ({ data["rule"] })', pad=20)

	color = 'tab:blue'
	color2 = 'tab:red'
	color_p = 'tab:cyan'

	ax.plot(total_probas, label="total proba", color=color)
	ax.plot(ratios, label="ratio of graph", color = color_p)
	ax.legend()
	ax.set_ylabel("proba", color=color)
	ax.tick_params(axis='y', labelcolor=color)

	# number of graphs
	ax2 = ax.twinx()
	ax2.plot(num_graphs, color=color2)
	ax2.set_yscale('log')
	ax2.set_ylabel("total number of graphs", color=color2)
	ax2.tick_params(axis='y', labelcolor=color2)

	fig.savefig("plots/stats/" + name)

	# graph sizes
	fig = plt.figure()
	ax = plt.axes()
	ax.set_xlabel('iterations')
	ax.set_ylabel('sizes')
	ax.set_title(f'graph average size and density ({ data["rule"] })', pad=20)
	ax.set_ylim(0, max(avg_size + std_dev_size)*1.1)

	ax.errorbar(iterations_list, avg_size, std_dev_size,
							capsize=2, elinewidth=1, markeredgewidth=2, label="average size", color=color)

	# graph densities
	ax2 = ax.twinx()
	ax2.set_ylabel('densities')
	ax2.set_ylabel("average density", color=color2)
	ax2.tick_params(axis='y', labelcolor=color2)
	ax2.set_ylim(0, 1)

	ax2.errorbar(iterations_list, avg_density, std_dev_density,
							capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)


	fig.savefig("plots/sizes/" + name)