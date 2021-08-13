#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
from matplotlib import cm
import numpy as np
import os
import sys

width = 0.7
multi_threading = False

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
	inverse_scaling[i, -1] = data["results"][n_thread]["cpu"]["total"] / data["results"]["1"]["cpu"]["total"]

	for step in range(proportions.shape[1]):
		proportions[i, step] = data["results"][n_thread]["wall"]["steps"][step] / data["results"][n_thread]["wall"]["total"]
		scaling[i, step] = data["results"]["1"]["wall"]["steps"][step] / data["results"][n_thread]["wall"]["steps"][step]
		inverse_scaling[i, step] = data["results"][n_thread]["cpu"]["steps"][step] / data["results"]["1"]["cpu"]["steps"][step]



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




fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 13), constrained_layout=True)

# plot wall time scalling
ax1.set_title("execution time reduction (in wall time) for each step")

ax1.set_xticks(tick_position)
ax1.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax1.bar(bar_starting_position + bar_width*step, scaling[:, step], width=bar_width, label=f'step { step + 1 }')

bars = ax1.bar(bar_starting_position + bar_width*proportions.shape[1], scaling[:, -1], width=bar_width, label='total')

ax1.plot(x_points[0], y_points[0], "k--")
for i in range(1, len(x_points)):
	ax1.plot(x_points[i], y_points[i], "--", color="dimgrey")
ax1.set_yscale('log')

ax1.legend()



# plot proportions
ax2.set_title("proportion of the total execution cpu time taken by each step")

ax2.set_xticks(tick_position_1)
ax2.set_xticklabels(n_threads)

for step in range(proportions.shape[1]):
	ax2.bar(bar_starting_position_1 + bar_width_1*step, proportions[:, step], width=bar_width_1, label=f'step { step + 1 }')



# plot cpu time scalling
ax3.set_title("execution time multiplication (in cpu time) for each step")

ax3.set_xticks(tick_position)
ax3.set_xticklabels(n_threads)

for step in range(scaling.shape[1]):
	ax3.bar(bar_starting_position + bar_width*step, inverse_scaling[:, step], width=bar_width, label=f'step { step + 1 }')

ax3.plot([total_begin, total_end], [1, 1], "k--")
ax3.set_yscale('log')


# saving fig
fig.suptitle(fig_title)
fig.savefig("plots/scaling.png")