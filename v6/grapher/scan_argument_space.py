#!/usr/bin/python3

import argparse
import os
import json

parser = argparse.ArgumentParser(description='run quantum iteration for all possible arguments')
parser.add_argument('-n', '--n-iter', type=int, default=10, help='number of iteration for each simulation')

parser.add_argument('--n-teta', type=int, default=10, help='number of teta scaned')
parser.add_argument('--n-phi', type=int, default=10, help='number of phi scaned')

parser.add_argument('--t0', '--teta-start', type=float, default=0, help='first teta (as a fraction of pi)')
parser.add_argument('--t1', '--teta-end', type=float, default=.5, help='last teta (as a fraction of pi)')

parser.add_argument('--p0', '--phi-start', type=float, default=0, help='first phi (as a fraction of pi)')
parser.add_argument('--p1', '--phi-end', type=float, default=.1, help='last phi (as a fraction of pi)')

parser.add_argument('args', nargs='*', default=[], help='last phi (as a fraction of pi)')

args = parser.parse_args()

teta = args.t0
phi = args.p0

cmd = f"./quantum_iterations.out --only-serialize-last-iter -n { args.n_iter } -t { teta } -p { phi } " + " ".join(args.args)

stream = os.popen(cmd)
output = json.loads(stream.read())

print(output)