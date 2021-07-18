#!/usr/bin/python3

# import utils
import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import utils

import json
import numpy as np

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for filename in filenames:
	with open(filename) as f:
		data = json.load(f)

	rule_name = utils.rule_name(data)
	name = utils.find_name("../../plots/sizes/", "probabilist_", rule_name)

	x_name = 'p'
	y_name = 'q'

	if data["rules"][0]["name"] == "split_merge":
		x_name += ' = P("split")'
		y_name += ' = P("merge")'
	elif data["rules"][0]["name"] == "erase_create":
		x_name += ' = P("erase")'
		y_name += ' = P("create")'

	ps = np.array(data["p"])
	qs = np.array(data["q"])

	size = np.zeros((len(ps), len(qs)))
	size_std_dev = np.zeros((len(ps), len(qs)))

	density = np.zeros((len(ps), len(qs)))
	density_std_dev = np.zeros((len(ps), len(qs)))

	for result in data["results"]:
		i = np.where(ps == result["p"])[0][0]
		j = np.where(qs == result["q"])[0][0]

		size[i, j] = result["data"]["avg_size"]
		size_std_dev[i, j] = result["data"]["std_dev_size"]

		density[i, j] = result["data"]["avg_density"]
		density_std_dev[i, j] = result["data"]["std_dev_density"]

	ps, qs = np.meshgrid(ps, qs)

	# plot size
	fig, ax1, ax2 = utils.plot_side_by_side(ps, qs, size)

	ax1.set_xlabel(x_name)
	ax2.set_xlabel(x_name)

	ax1.set_ylabel(y_name)
	ax2.set_ylabel(y_name)

	fig.suptitle(f'size after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/sizes/" + name)

	# plot density
	fig, ax1, ax2 = utils.plot_side_by_side(ps, qs, density)

	ax1.set_xlabel(x_name)
	ax2.set_xlabel(x_name)

	ax1.set_ylabel(y_name)
	ax2.set_ylabel(y_name)

	fig.suptitle(f'density after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/density/" + name)