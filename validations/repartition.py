#!/usr/bin/python3

from matplotlib import pyplot as plt 
import numpy as np
import random
from scipy.interpolate import interp1d

n_iter = 5000
n_graph = 10000
n_select = 4000
n_smooth = 20

def poly(lst, x):
	if len(lst) == 1:
		return lst[0]
	else:
		return poly(lst[1:], x)*x + lst[0]

def smooth(y):
	smoothed_y = np.zeros(len(y))

	for i in range(len(y)):
		begin = max(0, i - n_smooth//2)
		end = min(len(y), i + n_smooth//2)

		smoothed_y[i] = np.mean(y[begin:end])

	return smoothed_y

def selector_1(List, n):
	random_selector = np.random.rand(len(List))
	random_selector = np.log( - np.log(random_selector) / List)

	idx = np.argpartition(random_selector, n)
	return idx[:n]

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

def generate(selector, List):
	occurence = np.zeros(random_list.shape)

	for i in range(n_iter):
		occurence[selector(List, n_select)] += 1

	return occurence / n_iter

#random_list = np.concatenate((np.random.rand(n_graph), np.random.rand(n_graph) * 2, np.random.rand(n_graph) * 3))
#random_list = np.concatenate((np.random.rand(n_graph), np.random.rand(n_graph) * 2, -np.log(np.random.rand(n_graph))))
#x = 0.2; random_list = -np.log(np.random.rand(3*n_graph)*(1 - x) + x)
random_list = poly(np.random.rand(7), np.random.rand(3*n_graph))

random_list /= -np.sum(random_list)
random_list.sort()
random_list = -random_list

fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)

ax1.plot(smooth(generate(selector_1, random_list)), label="log(log())")
#ax1.plot(smooth(generate(selector_2, random_list)), label="histograms")

ax2 = ax1.twinx()
ax2.plot(random_list, c="r")

fig.legend()

fig.suptitle(f'random repartition and occurences')
fig.savefig("plots/repartition.jpg")
