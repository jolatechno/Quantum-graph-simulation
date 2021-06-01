from matplotlib import pyplot as plt
import sys
import numpy as np

def read_points(file):
    nums = file.read()
    return list(map(float, nums.split(',')))


for f_name in sys.argv[1:]:
    with open(f_name, 'r') as file:
        plots = read_points(file)
        plt.plot(np.arange(-len(plots) // 2, len(plots) // 2), plots, linewidth=1)

plt.savefig('graph.png')
