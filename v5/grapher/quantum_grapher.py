import json
from matplotlib import pyplot as plt 
import numpy as np

with open('res.json') as f:
  data = json.load(f)

n_iterations = len(data["iterations"])
n_max = max([len(it["nums"]) for it in data["iterations"]])

sizes = np.arange(0, n_max)
iterations = np.arange(0, n_iterations)

iterations, sizes = np.meshgrid(sizes, iterations)

total_proba = np.zeros(n_iterations)
total_num = np.zeros(n_iterations)

nums = np.zeros((n_iterations, n_max))
probas = np.zeros((n_iterations, n_max))

for i in range(n_iterations):
	probas_ = data["iterations"][i]["probas"]
	total_proba[i] = sum(probas_)
	probas_ /= total_proba[i]

	nums_ = data["iterations"][i]["nums"]
	total_num[i] = sum(nums_)
	nums_ /= total_num[i]

	for j in range(len(nums_)):
		nums[i, j] = nums_[j]
		probas[i, j] = probas_[j]

#print("sizes : ", sizes)
#print("\niterations ", iterations)
#print("\nnums ", nums)
#print("\nprobas ", probas)

fig = plt.figure()
ax = plt.axes(projection='3d')

ax.plot_surface(iterations, sizes, probas, cmap='viridis', edgecolor='none')
ax.set_title(f'graph probabilty ({ data["rule"] })')
ax.set_ylabel('iterations')
ax.set_xlabel('graph size')

fig.savefig(f'quantum_graphs_probabilities_{ data["rule"] }_{ data["initial state"]["size"] }.png')

fig = plt.figure()
ax = plt.axes(projection='3d')

ax.plot_surface(iterations, sizes, nums, cmap='viridis', edgecolor='none')
ax.set_title(f'number of graphs ({ data["rule"] })')
ax.set_ylabel('iterations')
ax.set_xlabel('graph size')

fig.savefig(f'quantum_graphs_nums_{ data["rule"] }_{ data["initial state"]["size"] }.png')

fig = plt.figure()
ax = plt.axes()
ax.set_xlabel('iterations')
ax.set_title(f'graph probabilty ({ data["rule"] })')

color = 'tab:blue'
color2 = 'tab:red'

ax.plot(total_proba, label="total proba", color=color)
ax.legend()
ax.set_ylabel("proba", color=color)
ax.tick_params(axis='y', labelcolor=color)

ax2 = ax.twinx()
ax2.plot(total_num, color=color2)
ax2.set_ylabel("total number of graphs", color=color2)
ax2.tick_params(axis='y', labelcolor=color2)


fig.savefig(f'quantum_graphs_stats_{ data["rule"] }_{ data["initial state"]["size"] }.png')