#!/usr/bin/python3

import json
from matplotlib import pyplot as plt 
import numpy as np

with open('res.json') as f:
  data = json.load(f)

n_iterations = len(data["iterations"])
n_max = max([len(it["nums"]) for it in data["iterations"]])

# lists

sizes_list = np.arange(0, n_max)
iterations_list = np.arange(0, n_iterations)

iterations, sizes = np.meshgrid(sizes_list, iterations_list)

total_proba = np.zeros(n_iterations)
total_num = np.zeros(n_iterations)
avg_size = np.zeros(n_iterations)
std_dev_size = np.zeros(n_iterations)

nums = np.zeros((n_iterations, n_max))
probas = np.zeros((n_iterations, n_max))

# compute values

for i in range(n_iterations):
	# compute proba
	probas_ = data["iterations"][i]["probas"]
	total_proba[i] = sum(probas_)
	probas_ /= total_proba[i]

	# compute nums
	nums_ = data["iterations"][i]["nums"]
	total_num[i] = sum(nums_)
	nums_ /= total_num[i]

	#compute sizes
	sizes_ = np.arange(0, len(nums_))
	avg_size[i] = sum(probas_ * sizes_)
	std_dev_size[i] = np.sqrt(abs(avg_size[i] * avg_size[i] - sum(probas_ * sizes_ * sizes_)))

	for j in range(len(nums_)):
		nums[i, j] = nums_[j]
		probas[i, j] = probas_[j]

#print("sizes : ", sizes)
#print("\niterations ", iterations)
#print("\nnums ", nums)
#print("\nprobas ", probas)


# graph probabilities

fig = plt.figure()
ax = plt.axes(projection='3d')

ax.plot_surface(iterations, sizes, probas, cmap='viridis', edgecolor='none')
ax.set_title(f'graph probabilty ({ data["rule"] })')
ax.set_ylabel('iterations')
ax.set_xlabel('graph size')
ax.set_zlabel('probability')

fig.savefig(f'plots/probabilities/p_{ data["rule"] }_{ data["initial state"]["size"] }.png')


# number of graphs 

fig = plt.figure()
ax = plt.axes(projection='3d')

ax.plot_surface(iterations, sizes, nums, cmap='viridis', edgecolor='none')
ax.set_title(f'number of graphs ({ data["rule"] })')
ax.set_ylabel('iterations')
ax.set_xlabel('graph size')
ax.set_zlabel('number of graphs')

fig.savefig(f'plots/proportions/n_{ data["rule"] }_{ data["initial state"]["size"] }.png')


# graph statistic
# probability
fig = plt.figure()
ax = plt.axes()
ax.set_xlabel('iterations')
ax.set_title(f'total probabilty and number of graph ({ data["rule"] })')

color = 'tab:blue'
color2 = 'tab:red'

ax.plot(total_proba, label="total proba", color=color)
ax.legend()
ax.set_ylabel("proba", color=color)
ax.tick_params(axis='y', labelcolor=color)

# number of graphs
ax2 = ax.twinx()
ax2.plot(total_num, color=color2)
ax2.set_ylabel("total number of graphs", color=color2)
ax2.tick_params(axis='y', labelcolor=color2)

fig.savefig(f'plots/stats/s_{ data["rule"] }_{ data["initial state"]["size"] }.png')


#graph sizes

fig = plt.figure()
ax = plt.axes()
ax.set_xlabel('iterations')
ax.set_ylabel('sizes')
ax.set_title(f'graph average size ({ data["rule"] })')
ax.errorbar(iterations_list, avg_size, std_dev_size,
						capsize=2, elinewidth=1, markeredgewidth=2)

fig.savefig(f'plots/sizes/as_{ data["rule"] }_{ data["initial state"]["size"] }.png')