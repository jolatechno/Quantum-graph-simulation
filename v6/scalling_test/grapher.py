#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import sys

width = 0.7

filename = "res.json" if len(sys.argv) == 1 else sys.argv[1]

with open(filename) as f:
	data = json.load(f)

n_threads = data["results"].keys()
fig_title = data["command"]

scaling = np.zeros((len(n_threads), len(data["results"]["1"]["steps"]) + 1))
proportions = np.zeros((len(n_threads), len(data["results"]["1"]["steps"])))

for i, n_thread in enumerate(n_threads):
	scaling[i, -1] = data["results"]["1"]["total"] / data["results"][n_thread]["total"]

	for step in range(proportions.shape[1]):
		proportions[i, step] = data["results"][n_thread]["steps"][step] / data["results"][n_thread]["total"]

		step_time = data["results"][n_thread]["steps"][step]
		scaling[i, step] = data["results"]["1"]["steps"][step] / step_time if step_time > 0 else 1

''' !!!TODO
add figure saving 
https://matplotlib.org/stable/gallery/lines_bars_and_markers/barchart.html !!! '''
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), constrained_layout=True)

# plot scalling
ax1.set_title("scalling for each step")

bar_width = width / proportions.shape[1]
tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
bar_starting_position = tick_position - bar_width * proportions.shape[1] / 2

ax1.set_xticks(tick_position)
ax1.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax1.bar(bar_starting_position + bar_width*step, scaling[:, step], width=bar_width, label=f'step { step + 1 }')

bars = ax1.bar(bar_starting_position + bar_width*proportions.shape[1], scaling[:, -1], width=bar_width, label='total')

ax1.legend()

# plot proportions
ax2.set_title("proportion of the total execution time taken by each step")

bar_width = width / (proportions.shape[1] - 1)
tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
bar_starting_position = tick_position - bar_width * (proportions.shape[1] - 1) / 2

ax2.set_xticks(tick_position)
ax2.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax2.bar(bar_starting_position + bar_width*step, proportions[:, step], width=bar_width, label=f'step { step + 1 }')

fig.suptitle(fig_title)
fig.savefig("plots/scalling.png")