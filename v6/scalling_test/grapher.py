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

n_threads = list(data["results"].keys())[::-1]
fig_title = data["command"]

scaling = np.zeros((len(n_threads), len(data["results"]["1"]["wall"]["steps"]) + 1))
inverse_scaling = np.zeros((len(n_threads), len(data["results"]["1"]["wall"]["steps"]) + 1))
proportions = np.zeros((len(n_threads), len(data["results"]["1"]["wall"]["steps"])))

for i, n_thread in enumerate(n_threads):
	scaling[i, -1] = data["results"]["1"]["wall"]["total"] / data["results"][n_thread]["wall"]["total"]

	for step in range(proportions.shape[1]):
		proportions[i, step] = data["results"][n_thread]["wall"]["steps"][step] / data["results"][n_thread]["wall"]["total"]
		scaling[i, step] = data["results"]["1"]["wall"]["steps"][step] / data["results"][n_thread]["wall"]["steps"][step]
		inverse_scaling[i, step] = data["results"][n_thread]["cpu"]["steps"][step] / data["results"]["1"]["cpu"]["steps"][step]

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 13), constrained_layout=True)

# plot wall time scalling
ax1.set_title("execution time reduction (in wall time) for each step")

bar_width = width / proportions.shape[1]
tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
bar_starting_position = tick_position - bar_width * proportions.shape[1] / 2

ax1.set_xticks(tick_position)
ax1.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax1.bar(bar_starting_position + bar_width*step, scaling[:, step], width=bar_width, label=f'step { step + 1 }')

bars = ax1.bar(bar_starting_position + bar_width*proportions.shape[1], scaling[:, -1], width=bar_width, label='total')

x_points, y_points = [], []
for i, n_thread in enumerate(n_threads):
	n_thread = int(n_thread)

	x_points.append(bar_starting_position[i])
	y_points.append(int(n_thread))

	x_points.append(bar_starting_position[i] + bar_width*proportions.shape[1])
	y_points.append(int(n_thread))
ax1.plot(x_points, y_points, "k--")

ax1.legend()



# plot proportions
ax2.set_title("proportion of the total execution cpu time taken by each step")

bar_width = width / (proportions.shape[1] - 1)
tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
bar_starting_position = tick_position - bar_width * (proportions.shape[1] - 1) / 2

ax2.set_xticks(tick_position)
ax2.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax2.bar(bar_starting_position + bar_width*step, proportions[:, step], width=bar_width, label=f'step { step + 1 }')



# plot cpu time scalling
ax3.set_title("execution time multiplication (in cpu time) for each step")

bar_width = width / proportions.shape[1]
tick_position = np.arange(len(n_threads), dtype=float)  # the label locations
bar_starting_position = tick_position - bar_width * proportions.shape[1] / 2

ax3.set_xticks(tick_position)
ax3.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax3.bar(bar_starting_position + bar_width*step, inverse_scaling[:, step], width=bar_width, label=f'step { step + 1 }')

bars = ax3.bar(bar_starting_position + bar_width*proportions.shape[1], inverse_scaling[:, -1], width=bar_width, label='total')
ax3.plot(ax3.get_xlim(), [1, 1], "k--")



# saving fig
fig.suptitle(fig_title)
fig.savefig("plots/scalling.png")