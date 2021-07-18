import os
import numpy as np

try:
	from matplotlib import pyplot as plt 
	from matplotlib import cm
except:
	pass

def rule_name(data):
	rule = ""

	n_rule = len(data["rules"])
	for i in range(n_rule):
		rule += data["rules"][i]["name"]

		if data["rules"][i]["move"]:
			rule += "_move"

		n_iter = data["rules"][i]["n_iter"]
		if n_iter > 1:
			rule += _ + str(1)

		if i < n_rule - 1:
			rule += "_"

	return rule

def find_name(dir, name_start, base_name):
	for i in range(1000000):
		name = name_start + f"{ i }_" + base_name + ".png"
		if not os.path.exists(dir + name):
			return name

class OPEN_LIST:
	pass

def print_to_json(indent, data, first_indent=True, braces=True):
	def print_value(indent, value):
		if isinstance(value, str):
			print("\"" + value + "\"", end='')

		elif isinstance(value, bool):
			print("true" if value else "false", end='')

		elif isinstance(value, list):
			print("[")

			for i in range(len(value)):

				print('\t'*(indent + 1), end='')
				print_value(indent + 1, value[i])
				print(',' if i != len(value) - 1 else '')

			print('\t'*indent + "]", end='')

		elif isinstance(value, dict):
			print_to_json(indent, value, False)

		elif isinstance(value, OPEN_LIST):
			print("[", end='')

		else:
			print(value, end='')

	if braces:
		print('\t'*indent*first_indent + "{")
	else:
		indent -= 1

	indent_string = '\t'*indent

	for j, key in enumerate(data):
		separator = ',' if j != len(data) - 1 else ''

		print(f"\t{ indent_string }\"{ key }\" : ", end='')
		print_value(indent + 1, data[key])
		print(',' if j != len(data) - 1 else '')

	if braces:
		print('\t'*indent + "}", end = '')

	#flush
	print("", end='', flush=True)

def plot_side_by_side(x, y, z):
	fig = plt.figure(figsize=plt.figaspect(0.5))

	ax1 = fig.add_subplot(1, 2, 1, projection='3d')

	if np.mean(z[:, 0]) < np.mean(z[:, -1]):
		ax1.set_xlim(ax1.get_xlim()[::-1])
	if np.mean(z[0, :]) > np.mean(z[-1, :]):
		ax1.set_ylim(ax1.get_ylim()[::-1])

	ax1.plot_surface(x, y, z, cmap=cm.coolwarm, linewidth=0, antialiased=False)

	ax2 = fig.add_subplot(1, 2, 2)
	pcm = ax2.pcolormesh(x, y, z, cmap=cm.coolwarm, shading='gouraud')

	fig.colorbar(pcm, ax=ax2)

	return fig, ax1, ax2
