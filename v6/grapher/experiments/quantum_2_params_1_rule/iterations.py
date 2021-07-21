#!/usr/bin/python3

# to get back the results use :
# scp -o ProxyJump=(user)@(adress_tunnel) (user)@(adress_computer):(directory)/Quantum-graph-simulation/v6/grapher/res.json ./

# import utils
import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import utils

import argparse
import numpy as np
import json




parser = argparse.ArgumentParser(description='run quantum iteration for all possible arguments')

parser.add_argument('-n', '--n-iter', type=int, default=10, help='number of iteration for each simulation')
parser.add_argument('-s', '--size', type=int, default=10, help='initial graph size')

parser.add_argument('-N', '--n-serializing', type=int, default=1, help='number of iteration for averaging values')

parser.add_argument('--n-theta', type=int, default=10, help='number of theta scaned')
parser.add_argument('--n-phi', type=int, default=10, help='number of phi scaned')

parser.add_argument('--t0', '--theta-start', type=float, default=0, help='first theta (as a fraction of pi)')
parser.add_argument('--t1', '--theta-end', type=float, default=.5, help='last theta (as a fraction of pi)')

parser.add_argument('--p0', '--phi-start', type=float, default=0, help='first phi (as a fraction of pi)')
parser.add_argument('--p1', '--phi-end', type=float, default=.1, help='last phi (as a fraction of pi)')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for quantum_iterations (put a space before "-" if you begin with a flag)')

args = parser.parse_args()

phis = list(np.linspace(args.p0, args.p1, args.n_phi))
thetas = list(np.linspace(args.t0, args.t1, args.n_theta))

def make_cmd(args, theta, phi):
	return f"../../quantum_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -s { args.size } -N -T 1e-18 -n { args.n_iter } -t { theta } -p { phi } --seed 0 " + " ".join(args.args)

# print rules
print("{")

n, args.n_iter = args.n_iter, 0

stream = os.popen(make_cmd(args, 0, 0))
rules = json.loads(stream.read())["rules"]

args.n_iter = n

for i in range(len(rules)):
	del rules[i]["theta"]
	del rules[i]["phi"]

utils.print_to_json(1,
	{
		"rules" : rules,
		"n_iter" : [args.n_iter - args.n_serializing, args.n_iter],
		"theta" : thetas,
		"phi" : phis,
		"results" : utils.OPEN_LIST()
	},
	True, False)

# iterate throught argument space
for i, theta in enumerate(thetas):
	for j, phi in enumerate(phis):
		stream = os.popen(make_cmd(args, theta, phi))
		data = stream.read()
		try:
			data = json.loads(data)
		except:
			print(data)
			raise

		# average over iterations
		avg = data["iterations"][0]
		for it in data["iterations"][1:]:
			for key in avg:
				avg[key] += it[key]

		# divided average by number of point
		for key in avg:
			avg[key] /= len(data["iterations"])

		avg["avg_size"] -= args.size

		# print to json
		utils.print_to_json(2,
			{
				"theta" : theta, 
				"phi" : phi, 
				"data" : avg
			}, 
			i == 0 and j == 0)

		if i != len(thetas) - 1 or j != len(phis) - 1:
			print(',', end = '')

# finish json   
print("\n\t]")
print("}")