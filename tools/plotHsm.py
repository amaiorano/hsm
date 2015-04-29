# Author: Antonio Maiorano (amaiorano@gmail.com)

import os
import sys
import tempfile

def PrintUsage():
	print """
Plots an HSM defined in cpp file(s) via hsmToDot -> dot -> default image viewer
Requires GraphViz 2.18 (Windows: http://www.graphviz.org/pub/graphviz/stable/windows/graphviz-2.18.exe)

Usage: {} <filespec>
	""".format(os.path.basename(sys.argv[0]))
	
def ExecCommand(command):
	print('[Exec] ' + command)
	os.system(command)

def main(argv = None):
	if argv is None:
		argv = sys.argv
	
	if len(argv) < 2:
		PrintUsage()
		return 0

	filespec = argv[1]
		
	# Write dot file
	dotFile = tempfile.gettempdir() + os.path.sep + os.path.basename(filespec) + '.dot'
	ExecCommand('hsmToDot.py ' + filespec + '>' + dotFile)
	
	# Invoke dot to produce image
	pngFile = dotFile + '.png'
	ExecCommand('dot ' + dotFile + ' -Tpng -o' + pngFile)
	
	# Open default image viewer
	ExecCommand(pngFile)

if __name__ == "__main__":
	sys.exit(main())
