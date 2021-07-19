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

parser.add_argument('--n-p', type=int, default=10, help='number of p scaned')
parser.add_argument('--n-q', type=int, default=10, help='number of q scaned')

parser.add_argument('--q0', '--q-start', type=float, default=0, help='first q for proabilist simulation')
parser.add_argument('--q1', '--q-end', type=float, default=1, help='last q for proabilist simulation')

parser.add_argument('--p0', '--p-start', type=float, default=0, help='first p for proabilist simulation')
parser.add_argument('--p1', '--p-end', type=float, default=1, help='last p for proabilist simulation')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for probabilist_iterations')

args = parser.parse_args()

ps = list(np.linspace(args.p0, args.p1, args.n_p))
qs = list(np.linspace(args.q0, args.q1, args.n_q))

def make_cmd(args, p, q):
	return f"../../probabilist_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -T 1e-18 -n { args.n_iter } -q { q } -p { p } --seed 0 " + " ".join(args.args)

# print rules
print("{")

n, args.n_iter = args.n_iter, 0

stream = os.popen(make_cmd(args, 0, 0))
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
		"q" : qs,
		"results" : utils.OPEN_LIST()
	},
	True, False)

# iterate throught argument space
for i, p in enumerate(ps):
	for j, q in enumerate(qs):

		stream = os.popen(make_cmd(args, p, q))
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

		# print to json
		utils.print_to_json(2,
			{
				"p" : p,
				"q" : q,
				"data" : avg
			},
			i == 0 and j == 0)

		if i != len(ps) - 1 or j != len(qs) - 1:
			print(',', end = '')

# finish json   
print("\n\t]")
print("}")