#!/usr/bin/python3

# import utils
import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import utils

import json
from matplotlib import pyplot as plt 
import numpy as np

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for filename in filenames:
	with open(filename) as f:
		data = json.load(f)

	rule_name = utils.rule_name(data)
	name = utils.find_name("../../plots/properties/", "probabilist_vs_quantum_", rule_name)

	ps = np.array(data["p"])
	tetas = np.array(data["teta"])

	ratios = [it["quantum"]["ratio"] for it in data["results"]]
	total_probas = [it["quantum"]["total_proba"] for it in data["results"]]

	probabilist_size = [it["probabilist"]["avg_size"] for it in data["results"]]
	probabilist_size_std_dev = [it["probabilist"]["std_dev_size"] for it in data["results"]]

	probabilist_density = [it["probabilist"]["avg_density"] for it in data["results"]]
	probabilist_density_std_dev = [it["probabilist"]["std_dev_density"] for it in data["results"]]

	quantum_size = [it["quantum"]["avg_size"] for it in data["results"]]
	quantum_size_std_dev = [it["quantum"]["std_dev_size"] for it in data["results"]]

	quantum_density = [it["quantum"]["avg_density"] for it in data["results"]]
	quantum_density_std_dev = [it["quantum"]["std_dev_density"] for it in data["results"]]

	fig = plt.figure(figsize=plt.figaspect(0.5), constrained_layout=True)

	y_lim = max(max(quantum_size + probabilist_size_std_dev), max(probabilist_size + probabilist_size_std_dev))*1.1

	color = 'tab:blue'
	color2 = 'tab:red'
	color_p = 'tab:cyan'

	x_label = f'p = P("{ data["rules"][0]["name"] }")'





	# graph quantum
	# graph sizes
	ax1 = fig.add_subplot(1, 2, 1)
	ax1.set_xlabel(x_label)
	ax1.set_ylabel('sizes')
	ax1.set_ylim(0, y_lim)

	ax1.errorbar(ps, quantum_size, quantum_size_std_dev,
				capsize=2, elinewidth=1, markeredgewidth=2, label="size variation", color=color)

	# graph densities
	ax1_2 = ax1.twinx()
	ax1_2.tick_params(axis='y', labelcolor=color2)
	ax1_2.set_ylim(0, 1)

	n_interval = len(ps) // 5
	ax1_2_x = ax1_2.twiny()
	ax1_2_x.set_xlim(ax1.get_xlim())
	ax1_2_x.set_xticks(ps[::n_interval])
	ax1_2_x.set_xticklabels(["%.2fπ" % teta for teta in tetas[::n_interval]])
	ax1_2_x.set_xlabel("teta")

	ax1_2.errorbar(ps, quantum_density, quantum_density_std_dev,
					capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)





	# graph probabilist
	# graph sizes
	ax2 = fig.add_subplot(1, 2, 2)
	ax2.set_xlabel(x_label)
	ax2.set_title(f'probabilist results', pad=20)
	ax2.set_ylim(0, y_lim)

	ax2.errorbar(ps, probabilist_size, probabilist_size_std_dev,
				capsize=2, elinewidth=1, markeredgewidth=2, label="size variation", color=color)

	# graph densities
	ax2_2 = ax2.twinx()
	ax2_2.set_ylabel("average density", color=color2)
	ax2_2.tick_params(axis='y', labelcolor=color2)
	ax2_2.set_ylim(0, 1)

	ax2_2.errorbar(ps, probabilist_density, probabilist_density_std_dev,
					capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)

	fig.suptitle(f'properties after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/properties/" + name)




	# probability
	fig = plt.figure(constrained_layout=True)
	ax = fig.add_subplot(1, 1, 1)
	ax.set_xlabel('iterations')

	n_interval = len(ps) // 5
	ax_x = ax.twiny()
	ax_x.set_xlim(ax1.get_xlim())
	ax_x.set_xticks(ps[::n_interval])
	ax_x.set_xticklabels(["%.2fπ" % teta for teta in tetas[::n_interval]])
	ax_x.set_xlabel("teta")

	ax.plot(ps, total_probas, label="total proba", color=color)
	ax.plot(ps, ratios, label="ratio of graph", color = color_p)
	ax.legend()
	ax.set_ylabel("proba", color=color)
	ax.tick_params(axis='y', labelcolor=color)

	fig.suptitle(f'stats after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/stats/" + name)