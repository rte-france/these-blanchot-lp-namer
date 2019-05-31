import os
import configparser

import sys

def parse_ini(root_path, output_path):
    #    read file and if not exists
	config = configparser.ConfigParser()
	config.read(root_path)
	outfile = open(output_path, 'w')
	for interco in config.sections():
		for kvp in config.items(interco):
			#{"1","name","pv"},
#			print interco, kvp
			if kvp [0] == 'link':
				paysor = kvp[1].split(' - ')[0].strip().replace(' ', '')
				paysex = kvp[1].split(' - ')[1].strip().replace(' ', '')
				outfile.write('%s %s %s\n' % (interco, "linkor", paysor.lower()))
				outfile.write('%s %s %s\n' % (interco, "linkex", paysex.lower()))
			else:
				outfile.write('%s %s %s\n' % (interco, kvp[0], kvp[1]))
	outfile.close()


root_path = None
output_path = None
if len(sys.argv)>2:
	root_path = os.path.join('.', sys.argv[1])
	output_path = os.path.join('.', sys.argv[2])
else:
	print("provide ini file and out file")
	sys.exit(0)
parse_ini(root_path, output_path)