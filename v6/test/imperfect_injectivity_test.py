#!/usr/bin/python3

# to get back the results use :
# scp -o ProxyJump=(user)@(adress_tunnel) (user)@(adress_computer):(directory)/Quantum-graph-simulation/v6/grapher/res.json ./

# import utils
import os, sys
import argparse
import numpy as np



parser = argparse.ArgumentParser(description='run imperfect injectivity test')

parser.add_argument('-n', '--n-iter', type=int, default=7, help='number of iteration for each simulation')

parser.add_argument('--rule', default="erase_create", help='rule for the simulation')

parser.add_argument('-s', '--start-size', type=int, default=13, help='minimum initial graph size')
parser.add_argument('-S', '--end-size', type=int, default=14, help='maximum initial graph size')

parser.add_argument('-r', '--start-seed', type=int, default=0, help='minimum seed')
parser.add_argument('-R', '--end-seed', type=int, default=10, help='maximum seed')

parser.add_argument('--t0', '--theta-start', type=float, default=0, help='first theta for quantum simulation')
parser.add_argument('--t1', '--theta-end', type=float, default=.25, help='last theta for quantum simulation')

parser.add_argument('--nt', '--num-theta', type=int, default=1, help='number of theta to test')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for probabilist_iterations')

args = parser.parse_args()

thetas = np.linspace(args.t0, args.t1, args.nt)
seeds = np.arange(args.start_seed, max(args.end_seed, args.start_seed + 1))
sizes = np.arange(args.start_size, max(args.end_size, args.start_size + 1))

def make_cmd(args, theta, size, seed):
	return f"./state_test.out --num-graph-print 1 -s { size } -n { args.n_iter } --theta { theta} --seed { seed } -r { args.rule } -i " + " ".join(args.args)


print("{")
print("\t\"points\" : [", end = '')

# iterate throught argument space
for i, theta in enumerate(thetas):
	for j, seed in enumerate(seeds):
		for k, size in enumerate(sizes):
			if i != 0 or j != 0 or k != 0:
				print(', ', end = '')

			stream = os.popen(make_cmd(args, theta, size, seed))
			data = stream.read().split("\n")

			initial_graph = " ".join(data[0].split(" ")[1:])
			final_graph = " ".join(data[2].split(" ")[1:])
			magnitude = complex(data[2].split(" ")[0].replace('i', 'j'))

			proba = 0 if initial_graph != final_graph else abs(magnitude)**2


			print(f"[{ data[1] }, { proba }]", end = '', flush=True)

# finish json   
print("]")
print("}")