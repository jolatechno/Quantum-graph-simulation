#!/usr/bin/python3 -W ignore

from matplotlib import pyplot as plt 
import numpy as np
import random
from scipy.interpolate import interp1d

n_iter = 1000
n_graph = 2000
n_smooth = 20

"""
useful functions :
"""

# generator for a random polynomial distribution
def poly(lst, x):
	if len(lst) == 1:
		return lst[0]
	else:
		return poly(lst[1:], x)*x + lst[0]

# linear smoothing for plotting
def smooth(y):
	smoothed_y = np.zeros(len(y))

	for i in range(len(y)):
		begin = max(0, i - n_smooth//2)
		end = min(len(y), i + n_smooth//2)

		smoothed_y[i] = np.mean(y[begin:end])

	return smoothed_y

# generating an occurence list from a selecting method and a random distribution (List)
def generate(selector, List, n_select):
	occurence = np.zeros(random_list.shape)

	for i in range(n_iter):
		occurence[selector(List, n_select)] += 1

	# normalizeing occurences so they idely match the distribution
	return occurence / n_iter / n_select



"""
tested method of selection :
"""

def selector_1(List, n):
	random_selector = np.random.rand(len(List))
	random_selector = np.log( - np.log(random_selector) / List)

	idx = np.argpartition(random_selector, n)
	return idx[:n]



"""
generating the random distribution :
"""

# starting with an empty list
random_list = np.ndarray([])

# adding uniform distributions
for rate in [1, 2, 4]:
	distribution = np.random.rand(n_graph) * rate
	distribution.sort()
	random_list = np.append(random_list, distribution)

# adding exponential distribution
for rate in [1, 2, 4]:
	distribution = -np.log((np.random.rand(n_graph)*(1 - 0.05) + 0.05)) / rate
	distribution.sort()
	random_list = np.append(random_list, distribution)

# adding an inverse polynomial distribution
distribution = poly(np.random.rand(7), np.random.rand(3*n_graph))
distribution.sort()
random_list = np.append(random_list, distribution)

# normalizeing the distribution
random_list /= np.sum(random_list)



"""
plotting the distribution and occurences for different proportions of graph selected :
"""

fig = plt.figure(figsize=(10, 10), constrained_layout=True)
ax = fig.add_subplot(1, 1, 1)

# setting up the frame
ax.set_ylabel("probability")
ax.set_yticks([])
ax.set_xticks([])

# plotting occurences for different proportions of graph selected
for proportion in [.1, .25, .5]:
	n_select = int(len(random_list) * proportion)
	ax.plot(smooth(generate(selector_1, random_list, n_select)), label=f"log(log()), { proportion * 100 }% selected", linewidth=2)
	#ax1.plot(smooth(generate(selector_2, random_list)), label="histograms")

# plotting the initial ditribution
ax.plot(random_list, "r--", label="base distribution", linewidth=3)

# adding separators for the different distributions
ax.set_ylim(ax.get_ylim())
ax.plot([3*n_graph, 3*n_graph], [-1, 1], "k--", linewidth=4)
ax.plot([6*n_graph, 6*n_graph], [-1, 1], "k--", linewidth=4)

# adding text to name different sections
y_max = ax.get_ylim()[1]
args = {
	"horizontalalignment" : 'center',
	"verticalalignment" : 'center',
	"bbox" : {
		"facecolor" : 'white',
		"alpha" : 0.5,
	}
}
ax.text(1.5*n_graph, y_max*.75, "linear distributions", **args)
ax.text(4.5*n_graph, y_max*.75, "exponential distributions", **args)
ax.text(7.5*n_graph, y_max*.75, "inverse polynomial distribution", **args)

fig.legend()

# saving the figure
fig.suptitle(f'random repartition and occurences')
fig.savefig("plots/repartition.jpg")

"""
# "histograms" selector
def selector_2(List, n):
	n_zone = n // 10
	idx = []

	csum = np.cumsum(List)

	# find the number of zones
	num_sample_per_zone = 0
	while True:
		num_sample_per_zone = int(np.ceil(n / n_zone))

		end = 0
		while csum[end] < 1 / n_zone:
			end += 1

		if end > num_sample_per_zone * 1.3:
			break

		n_zone -= 1

	# find each zone
	start, end = 0, 0
	for zone in range(n_zone):
		if zone == n_zone - 1:
			
			# select n distinct random index between i and len(List)
			idx += random.sample(range(start, len(List)), n - (n_zone - 1)*num_sample_per_zone)
		else:
			while csum[end] - csum[start] < 1/n_zone:
				end += 1

			# select n distinct random index between j and i
			idx += random.sample(range(start, end), num_sample_per_zone)

			start = end

	return np.array(idx)
"""