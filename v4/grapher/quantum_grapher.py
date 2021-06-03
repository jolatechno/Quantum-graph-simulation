import json
from matplotlib import pyplot as plt 
import numpy as np

with open('res.json') as f:
  data = json.load(f)

n_iterations = len(data["iterations"])
n_max = len(data["iterations"][-1]["nums"])

sizes = np.arange(0, n_max)
iterations = np.arange(0, n_iterations)

iterations, sizes = np.meshgrid(sizes, iterations)

nums = np.zeros((n_iterations, n_max))
probas = np.zeros((n_iterations, n_max))

for i in range(n_iterations):
	nums_ = data["iterations"][i]["nums"]
	probas_ = data["iterations"][i]["probas"]
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
ax.set_xlabel('iterations')
ax.set_ylabel('graph size')

fig.savefig(f'quantum_graphs_probabilities_{ data["rule"] }_{ data["initial state"]["size"] }.png')

fig = plt.figure()
ax = plt.axes(projection='3d')

ax.plot_surface(iterations, sizes, nums, cmap='viridis', edgecolor='none')
ax.set_title(f'number of graphs ({ data["rule"] })')
ax.set_xlabel('iterations')
ax.set_ylabel('graph size')

fig.savefig(f'quantum_graphs_nums_{ data["rule"] }_{ data["initial state"]["size"] }.png')