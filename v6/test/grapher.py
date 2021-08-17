#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import sys

filenames = ["res.json"] if len(sys.argv) == 1 else sys.argv[1:]

for filename in filenames:
	with open(filename) as f:
	  data = np.array(json.load(f)["points"])

	fig, ax = plt.subplots(1, 1, figsize=(10, 10), constrained_layout=True)
	ax.set_ylabel("sclar product between the initial and the final state")
	ax.set_xlabel("total probability including loss")

	ax.set_xlim([-0.01, 1.01])
	ax.set_ylim([-0.01, 1.01])

	ax.plot(data[:, 0], data[:, 1], "b+", markersize=5)

	fig.savefig("plots/scalar_product.png")
