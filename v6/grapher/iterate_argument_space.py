#!/usr/bin/python3

import argparse
import os
import numpy as np
import json

def print_to_json(indent, data):
	indent_string = '\t'*indent

	print(f"{ indent_string }{{")
	for j, key in enumerate(data):
		separator = ',' if j != len(data) - 1 else ''

		stringified = data[key]

		if isinstance(stringified, str):
			stringified = "\"" + stringified + "\""
		elif isinstance(stringified, bool):
			stringified = "true" if stringified else "false"

		print(f"\t{ indent_string }\"{ key }\" : { stringified }{ separator }")
	print(f"{ indent_string }}}{ ',' if i != len(rules) - 1 else '' }")

parser = argparse.ArgumentParser(description='run quantum iteration for all possible arguments')
parser.add_argument('-n', '--n-iter', type=int, default=10, help='number of iteration for each simulation')

parser.add_argument('-N', '--n-serializing', type=int, default=1, help='number of iteration for averaging values')

parser.add_argument('-r', '--start-seed', type=int, default=0, help='first seed tested')
parser.add_argument('-R', '--end-seed', type=int, default=1, help='last seed tested')

parser.add_argument('--n-teta', type=int, default=10, help='number of teta scaned')
parser.add_argument('--n-phi', type=int, default=10, help='number of phi scaned')

parser.add_argument('--t0', '--teta-start', type=float, default=0, help='first teta (as a fraction of pi)')
parser.add_argument('--t1', '--teta-end', type=float, default=.5, help='last teta (as a fraction of pi)')

parser.add_argument('--p0', '--phi-start', type=float, default=0, help='first phi (as a fraction of pi)')
parser.add_argument('--p1', '--phi-end', type=float, default=.1, help='last phi (as a fraction of pi)')

parser.add_argument('--args', nargs='+', default=[], help='cli arguments for quantum_iterations (put a space before "-" if you begin with a flag)')

args = parser.parse_args()

n_avg = (args.end_seed - args.start_seed) * args.n_serializing

phis = list(np.linspace(args.p0, args.p1, args.n_phi))
tetas = list(np.linspace(args.t0, args.t1, args.n_teta))

def make_cmd(args, teta, phi, seed):
	return f"./quantum_iterations.out --start-serializing { max(0, args.n_iter - args.n_serializing + 1) } -N -T 1e-18 -n { args.n_iter } -t { teta } -p { phi } --seed { seed } " + " ".join(args.args)

# print rules
print("{")
print("\t\"rules\" : [")

n, args.n_iter = args.n_iter, 0

stream = os.popen(make_cmd(args, 0, 0, 0))
rules = json.loads(stream.read())["rules"]

args.n_iter = n

for i in range(len(rules)):
	del rules[i]["teta"]
	del rules[i]["phi"]

	print_to_json(2, rules[i])

print("\t],", flush=True)
print(f"\t\"phi\" : { phis },")
print(f"\t\"teta\" : { tetas },")
print("\t\"results\" : [", flush=True)

# iterate throught argument space
for phi in phis:
	for teta in tetas:

		avg = None

		for seed in range(args.start_seed, args.end_seed):
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
		print("\t\t{", flush=True)
		print(f"\t\t\t\"teta\" : { teta },")
		print(f"\t\t\t\"phi\" : { phi },")
		print("\t\t\t\"data\" : {")
		print_to_json(3, avg)
		print(f"\t\t}}{ ',' if teta != tetas[-1] or phi != phis[-1] else '' }", flush=True)

# finish json   
print("\t]")
print("}")