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
	name = utils.find_name("../../plots/sizes/", "quantum_", rule_name)

	thetas = np.array(data["theta"])
	phis = np.array(data["phi"])

	size = np.zeros((len(thetas), len(phis)))
	size_std_dev = np.zeros((len(thetas), len(phis)))

	density = np.zeros((len(thetas), len(phis)))
	density_std_dev = np.zeros((len(thetas), len(phis)))

	total_proba = np.zeros((len(thetas), len(phis)))

	for result in data["results"]:
		i = np.where(thetas == result["theta"])[0][0]
		j = np.where(phis == result["phi"])[0][0]

		size[i, j] = result["data"]["avg_size"]
		size_std_dev[i, j] = result["data"]["std_dev_size"]

		density[i, j] = result["data"]["avg_density"]
		density_std_dev[i, j] = result["data"]["std_dev_density"]

		total_proba[i, j] = result["data"]["total_proba"]

	thetas, phis = np.meshgrid(thetas, phis)

	# plot size
	fig, ax1, ax2 = utils.plot_side_by_side(thetas, phis, size)

	ax1.set_xlabel("theta")
	ax2.set_xlabel("theta")

	ax1.set_ylabel("phi")
	ax2.set_ylabel("phi")

	fig.suptitle(f'size variation after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/sizes/" + name)

	# plot density
	fig, ax1, ax2 = utils.plot_side_by_side(thetas, phis, density)

	ax1.set_xlabel("theta")
	ax2.set_xlabel("theta")

	ax1.set_ylabel("phi")
	ax2.set_ylabel("phi")

	fig.suptitle(f'density after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/density/" + name)

	# plot total_proba
	fig, ax1, ax2 = utils.plot_side_by_side(thetas, phis, total_proba)

	ax1.set_xlabel("theta")
	ax2.set_xlabel("theta")

	ax1.set_ylabel("phi")
	ax2.set_ylabel("phi")

	fig.suptitle(f'total proba after { data["n_iter"] } iterations of { rule_name }')
	fig.savefig("../../plots/probas/" + name)