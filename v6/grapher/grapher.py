#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import sys

from experiments import utils

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for filename in filenames:
	with open(filename) as f:
	  data = json.load(f)

	rule_name = utils.rule_name(data)
	theta = round(data["rules"][0]["theta"] / np.pi, 2)
	phi = round(data["rules"][0]["phi"] / np.pi, 2)

	probabilist = "p" in data["rules"][0]
	name = utils.find_name("plots/stats/", "probabilist_" if probabilist else "quantum_", rule_name)

	n_iterations = data["n_iter"] + 1
	iterations_list = np.arange(0, n_iterations)

	total_probas = np.array([it["total_proba"] for it in data["iterations"]])
	ratios = np.array([it["ratio"] for it in data["iterations"]])
	num_graphs = np.array([it["num_graphs"] for it in data["iterations"]])

	avg_size = np.array([it["avg_size"] for it in data["iterations"]])
	std_dev_size = np.array([it["std_dev_size"] for it in data["iterations"]])

	avg_density = np.array([it["avg_density"] for it in data["iterations"]])
	std_dev_density = np.array([it["std_dev_density"] for it in data["iterations"]])

	color = 'tab:blue'
	color2 = 'tab:red'
	color_p = 'tab:cyan'

	if not probabilist:
		# probability
		fig = plt.figure(constrained_layout=True)
		ax = fig.add_subplot(1, 1, 1)
		ax.set_xlabel('iterations')
		ax.set_title(f'total probabilty and number of graph', pad=20)

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

		fig.suptitle(f'{ rule_name }, θ={ theta }π, φ={ phi }π\n')
		fig.savefig("plots/stats/" + name)

	# graph sizes
	fig = plt.figure(constrained_layout=True)
	ax = fig.add_subplot(1, 1, 1)
	ax.set_xlabel('iterations')
	ax.set_ylabel('sizes')
	ax.set_title(f'graph average size and density', pad=20)
	ax.set_ylim(0, max(avg_size + std_dev_size)*1.1)

	ax.errorbar(iterations_list, avg_size, std_dev_size,
				capsize=2, elinewidth=1, markeredgewidth=2, label="average size", color=color)

	# graph densities
	ax2 = ax.twinx()
	ax2.set_ylabel("average density", color=color2)
	ax2.tick_params(axis='y', labelcolor=color2)
	ax2.set_ylim(0, 1)

	ax2.errorbar(iterations_list, avg_density, std_dev_density,
				capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)

	fig.suptitle(f'{ rule_name }, θ={ theta }π, φ={ phi }π\n')
	fig.savefig("plots/properties/" + name)
