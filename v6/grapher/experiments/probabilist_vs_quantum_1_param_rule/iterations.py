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

parser.add_argument('--n-p', type=int, default=10, help='number of p scaned')

parser.add_argument('--p0', '--p-start', type=float, default=0, help='first p for proabilist simulation')
parser.add_argument('--p1', '--p-end', type=float, default=1, help='last p for proabilist simulation')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for both quantum_iterations and probabilist_iterations')
parser.add_argument('--p-args', nargs='+', default=[], help='cli arguments for probabilist_iterations')
parser.add_argument('--q-args', nargs='+', default=[], help='cli arguments for quantum_iterations')

args = parser.parse_args()

ps = np.linspace(args.p0, args.p1, args.n_p)
tetas = np.arccos(np.sqrt(1 - ps)) / np.pi

ps, tetas = list(ps), list(tetas)

def make_probabilist_cmd(args, p):
	return f"../../probabilist_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -s { args.size } -T 1e-18 -n { args.n_iter } -p { p } -q { p } --seed 0 " + " ".join(args.args + args.p_args)

def make_quantum_cmd(args, teta):
	return f"../../quantum_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -s { args.size } -N -T 1e-18 -n { args.n_iter } --teta { teta } --phi 0 --seed 0 " + " ".join(args.args + args.q_args)

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
		"teta" : tetas,
		"results" : utils.OPEN_LIST()
	},
	True, False)

# iterate throught argument space
for i, (p, teta) in enumerate(zip(ps, tetas)):



	# read probabilist result
	stream = os.popen(make_probabilist_cmd(args, p))
	data = stream.read()
	try:
		data = json.loads(data)
	except:
		print(data)
		raise

	# average over iterations
	probabilist_avg = data["iterations"][0]
	for it in data["iterations"][1:]:
		for key in probabilist_avg:
			probabilist_avg[key] += it[key]

	# divided average by number of point
	for key in probabilist_avg:
		probabilist_avg[key] /= len(data["iterations"])

	probabilist_avg["avg_size"] -= args.size
	



	# read quantum result
	stream = os.popen(make_quantum_cmd(args, teta))
	data = stream.read()
	try:
		data = json.loads(data)
	except:
		print(data)
		raise

	# average over iterations
	quantum_avg = data["iterations"][0]
	for it in data["iterations"][1:]:
		for key in quantum_avg:
			quantum_avg[key] += it[key]

	# divided average by number of point
	for key in quantum_avg:
		quantum_avg[key] /= len(data["iterations"])

	quantum_avg["avg_size"] -= args.size




	# print to json
	utils.print_to_json(2, 
		{
			"p" : p,
			"teta" : teta,
			"probabilist" : probabilist_avg,
			"quantum" :  quantum_avg
		}, 
		i == 0)

	if i != len(ps) - 1:
		print(',', end = '')

# finish json   
print("\n\t]")
print("}")