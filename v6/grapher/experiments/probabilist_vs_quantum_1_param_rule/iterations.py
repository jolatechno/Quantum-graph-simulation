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

parser.add_argument('-N', '--n-serializing', type=int, default=1, help='number of iteration for averaging values')

parser.add_argument('-r', '--start-seed', type=int, default=0, help='first seed tested')
parser.add_argument('-R', '--end-seed', type=int, default=1, help='last seed tested')

parser.add_argument('--n-p', type=int, default=10, help='number of p scaned')

parser.add_argument('--p0', '--p-start', type=float, default=0, help='first p for proabilist simulation')
parser.add_argument('--p1', '--p-end', type=float, default=1, help='last p for proabilist simulation')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for probabilist_iterations (put a space before "-" if you begin with a flag)')

args = parser.parse_args()

seeds = range(args.start_seed, args.end_seed)
n_avg = (args.end_seed - args.start_seed) * args.n_serializing

ps = list(np.linspace(args.p0, args.p1, args.n_p))

def make_probabilist_cmd(args, p):
	return f"../../probabilist_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -T 1e-18 -n { args.n_iter } -p { p } -q { p } --seed 0" + " ".join(args.args)

def make_quantum_cmd(args, p, seed):
	teta = np.arccos(np.sqrt(1 - p))
	return f"../../quantum_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -N -T 1e-18 -n { args.n_iter } --teta { teta } --phi 0 --seed { seed }" + " ".join(args.args)

# print rules
print("{")

n, args.n_iter = args.n_iter, 0

stream = os.popen(make_probabilist_cmd(args, 0))
rules = json.loads(stream.read())["rules"]

args.n_iter = n

for i in range(len(rules)):
	del rules[i]["p"]
	del rules[i]["q"]

utils.print_to_json(1,
	{
		"rules" : rules,
		"n_iter" : [args.n_iter - args.n_serializing, args.n_iter],
		"p" : ps,
		"results" : utils.OPEN_LIST()
	},
	True, False)

# iterate throught argument space
for i, p in enumerate(ps):



	# read probabilist result
	stream = os.popen(make_probabilist_cmd(args, p))
	data = json.loads(stream.read())

	# average over iterations
	probabilist_avg = data["iterations"][0]
	for it in data["iterations"][1:]:
		for key in probabilist_avg:
			probabilist_avg[key] += it[key]

	# divided average by number of point
	for key in probabilist_avg:
		probabilist_avg[key] /= args.n_serializing




	# read quantum result
	quantum_avg = None

	for seed in seeds:
		stream = os.popen(make_quantum_cmd(args, p, seed))
		data = json.loads(stream.read())

		# average over iterations
		for it in data["iterations"]:
			if quantum_avg is None:
				quantum_avg = it
			else:
				for key in quantum_avg:
					quantum_avg[key] += it[key]

	# divided average by number of point
	for key in quantum_avg:
		quantum_avg[key] /= n_avg




	# print to json
	utils.print_to_json(2, {"p" : p, "probabilist" : probabilist_avg, "quantum" :  quantum_avg}, i == 0)

	if i != len(ps) - 1:
		print(',', end = '')

# finish json   
print("\n\t]")
print("}")