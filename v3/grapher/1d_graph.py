from matplotlib import pyplot as plt
import sys

def read_points(file):
    nums = file.read()
    return [[int(num) for num in nums_1d.split(',')] for nums_1d in nums.split(';')]

for f_name in sys.argv[1:]:
    with open(f_name, 'r') as file:
        plots = read_points(file)
        for plot in plots:
            plt.plot(plot, linewidth=1)

plt.savefig('graph.png')
