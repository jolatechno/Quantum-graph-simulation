#!/usr/bin/python3

import argparse
import os
import numpy as np
import json

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

rules = None

print("{")
print("\t\"rules\" : [")

for phi in phis:
	for teta in tetas:

		avg = None

		for seed in range(args.start_seed, args.end_seed):
			cmd = f"./quantum_iterations.out --start-serializing { args.n_iter - args.n_serializing + 1 } -N -T 1e-18 -n { args.n_iter } -t { teta } -p { phi } --seed { seed } " + " ".join(args.args)

			stream = os.popen(cmd)
			data = json.loads(stream.read())

			# add rules to json
			if rules is None:
				rules = data["rules"]

				for i in range(len(rules)):
					del rules[i]["teta"]
					del rules[i]["phi"]

				for i, rule in enumerate(rules):
					print("\t\t{")
					for j, key in enumerate(rule):
						print(f"\t\t\t\"{ key }\" : { rule[key] }{ ',' if i != len(rule) - 1 else '' }")
					print(f"\t\t{ '},' if i != len(rules) - 1 else '}' }")

				print("\t],")
				print(f"\t\"phi\" : { phis },")
				print(f"\t\"teta\" : { tetas },")
				print("\t\"results\" : [")

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
		print("\t\t{")
		print(f"\t\t\t\"teta\" { teta },")
		print(f"\t\t\t\"phi\" { phi },")
		print("\t\t\t\"data\" : {")
		for i, key in enumerate(avg):
			print(f"\t\t\t\t\"{ key }\" : { avg[key] }{ ',' if i != len(avg) - 1 else '' }")
		print("\t\t\t}")
		print(f"\t\t{ '},' if teta != tetas[-1] or phi != phis[-1] else '}' }")

# Serializing json   
print("\t]")
print("}")