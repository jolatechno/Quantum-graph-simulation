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

	probabilist_size = np.zeros(len(ps))
	probabilist_size_std_dev = np.zeros(len(ps))

	quantum_size = np.zeros(len(ps))
	quantum_size_std_dev = np.zeros(len(ps))

	probabilist_density = np.zeros(len(ps))
	probabilist_density_std_dev = np.zeros(len(ps))

	quantum_density = np.zeros(len(ps))
	quantum_density_std_dev = np.zeros(len(ps))

	for i, it in enumerate(data["results"]):
		probabilist_size[i] = it["probabilist"]["avg_size"]
		probabilist_size_std_dev[i] = it["probabilist"]["std_dev_size"]

		probabilist_density[i] = it["probabilist"]["avg_density"]
		probabilist_density_std_dev[i] = it["probabilist"]["std_dev_density"]

		quantum_size[i] = it["quantum"]["avg_size"]
		quantum_size_std_dev[i] = it["quantum"]["std_dev_size"]

		quantum_density[i] = it["quantum"]["avg_density"]
		quantum_density_std_dev[i] = it["quantum"]["std_dev_density"]

	fig = plt.figure(figsize=plt.figaspect(0.5))

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
	ax1.set_title(f'quantum results', pad=20)
	ax1.set_ylim(0, y_lim)

	ax1.errorbar(ps, quantum_size, quantum_size_std_dev,
				capsize=2, elinewidth=1, markeredgewidth=2, label="average size", color=color)

	# graph densities
	ax1_2 = ax1.twinx()
	ax1_2.tick_params(axis='y', labelcolor=color2)
	ax1_2.set_ylim(0, 1)

	ax1_2.errorbar(ps, quantum_density, quantum_density_std_dev,
					capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)





	# graph probabilist
	# graph sizes
	ax2 = fig.add_subplot(1, 2, 2)
	ax2.set_xlabel(x_label)
	ax2.set_title(f'probabilist results', pad=20)
	ax2.set_ylim(0, y_lim)

	ax2.errorbar(ps, probabilist_size, probabilist_size_std_dev,
				capsize=2, elinewidth=1, markeredgewidth=2, label="average size", color=color)

	# graph densities
	ax2_2 = ax2.twinx()
	ax2_2.set_ylabel("average density", color=color2)
	ax2_2.tick_params(axis='y', labelcolor=color2)
	ax2_2.set_ylim(0, 1)

	ax2_2.errorbar(ps, probabilist_density, probabilist_density_std_dev,
					capsize=2, elinewidth=1, markeredgewidth=2, label="average density", color=color2)

	n_interval = len(ps) // 5
	ax2_2_x = ax2_2.twiny()
	ax2_2_x.set_xlim(ax1.get_xlim())
	ax2_2_x.set_xticks(ps[::n_interval])
	ax2_2_x.set_xticklabels(["%.2f" % teta for teta in tetas[::n_interval]])
	ax2_2_x.set_xlabel("teta")

	fig.savefig("../../plots/properties/" + name)