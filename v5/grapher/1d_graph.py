from matplotlib import pyplot as plt
import sys
import numpy as np

with open("res.txt", 'r') as file:
    nums = file.read()
    plots = list(map(float, nums.split(',')))
    plt.plot(np.arange(-len(plots) // 2, len(plots) // 2), plots, linewidth=1)

plt.savefig('graph.png')
