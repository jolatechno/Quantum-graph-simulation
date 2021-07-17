#!/usr/bin/python3

# to get back the results use :
# scp -o ProxyJump=(user)@(adress_tunnel) (user)@(adress_computer):(directory)/Quantum-graph-simulation/v6/grapher/res.json ./

import argparse
import os
import numpy as np
import json

def print_to_json(indent, data, first_indent=True):
	indent_string = '\t'*indent

	print(f"{ indent_string if first_indent else '' }{{")
	for j, key in enumerate(data):
		separator = ',' if j != len(data) - 1 else ''

		stringified = data[key]

		if isinstance(stringified, str):
			stringified = "\"" + stringified + "\""
		elif isinstance(stringified, bool):
			stringified = "true" if stringified else "false"

		print(f"\t{ indent_string }\"{ key }\" : { stringified }{ separator }")
	print(f"{ indent_string }}}{ ',' if i != len(rules) - 1 else '' } ", end = '')

parser = argparse.ArgumentParser(description='run quantum iteration for all possible arguments')

parser.add_argument('-p', '--probabilist', default=False, action=argparse.BooleanOptionalAction, help='run probabilist simulation')
parser.add_argument('-n', '--n-iter', type=int, default=10, help='number of iteration for each simulation')

parser.add_argument('-N', '--n-serializing', type=int, default=1, help='number of iteration for averaging values')

parser.add_argument('-r', '--start-seed', type=int, default=0, help='first seed tested (discarded if probabilist)')
parser.add_argument('-R', '--end-seed', type=int, default=1, help='last seed tested (discarded if probabilist)')

parser.add_argument('--n-teta', type=int, default=10, help='number of teta scaned')
parser.add_argument('--n-phi', type=int, default=10, help='number of phi scaned')

parser.add_argument('--t0', '--q0', '--q-start', '--teta-start', type=float, default=0, help='first teta (as a fraction of pi), or first q for proabilist simulation')
parser.add_argument('--t1', '--q1', '--q-end', '--teta-end', type=float, default=.5, help='last teta (as a fraction of pi), or last q for proabilist simulation')

parser.add_argument('--p0', '--p-start', '--phi-start', type=float, default=0, help='first phi (as a fraction of pi), or first p for proabilist simulation')
parser.add_argument('--p1', '--p-end', '--phi-end', type=float, default=.1, help='last phi (as a fraction of pi), or last p for proabilist simulation')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for quantum_iterations (put a space before "-" if you begin with a flag)')

args = parser.parse_args()

n_avg = (args.end_seed - args.start_seed) * args.n_serializing

phis = list(np.linspace(args.p0, args.p1, args.n_phi))
tetas = list(np.linspace(args.t0, args.t1, args.n_teta))

seeds = [0] if args.probabilist else range(args.start_seed, args.end_seed)

def make_cmd(args, teta, phi, seed):
	if args.probabilist:
		return f"./probabilist_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -N 5 -T 1e-18 -n { args.n_iter } -q { teta } -p { phi } --seed 0" + " ".join(args.args)
	else:
		return f"./quantum_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -N -T 1e-18 -n { args.n_iter } -t { teta } -p { phi } --seed { seed } " + " ".join(args.args)

# print rules
print("{")
print("\t\"rules\" : [")

n, args.n_iter = args.n_iter, 0

stream = os.popen(make_cmd(args, 0, 0, 0))
rules = json.loads(stream.read())["rules"]

args.n_iter = n

for i in range(len(rules)):
	if args.probabilist:
		del rules[i]["p"]
		del rules[i]["q"]
	else:
		del rules[i]["teta"]
		del rules[i]["phi"]

	print_to_json(2, rules[i])

print("\n\t],")
print(f"\t\"{ 'p' if args.probabilist else 'phi' }\" : { phis },")
print(f"\t\"{ 'q' if args.probabilist else 'teta' }\" : { tetas },")
print(f"\t\"n_iter\" : [{ args.n_iter - args.n_serializing }, {  args.n_iter }],")
print("\t\"results\" : [", flush=True)

# iterate throught argument space
for phi in phis:
	for teta in tetas:

		avg = None

		for seed in seeds:
			stream = os.popen(make_cmd(args, teta, phi, seed))
			data = json.loads(stream.read())

			# average over iterations
			for it in data["iterations"]:
				if avg is None:
					avg = it
				else:
					for key in avg:
						avg[key] += it[key]

		# divided average by number of point
		for key in avg:
			avg[key] /= n_avg

		# print to json
		print("\t\t{" if teta == tetas[0] and phi == phis[0] else "{")
		print(f"\t\t\t\"{ 'q' if args.probabilist else 'teta' }\" : { teta },")
		print(f"\t\t\t\"{ 'p' if args.probabilist else 'phi' }\" : { phi },")
		print("\t\t\t\"data\" : ", end = '')
		print_to_json(3, avg, False)
		print(f"\n\t\t}}{ ',' if teta != tetas[-1] or phi != phis[-1] else '' }", flush=True, end = '')

# finish json   
print("\n\t]")
print("}")