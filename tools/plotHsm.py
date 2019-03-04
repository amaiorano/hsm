# Author: Antonio Maiorano (amaiorano@gmail.com)

import os
import sys
import tempfile

def PrintUsage():
	print """
Plots an HSM defined in cpp file(s) via hsmToDot -> dot -> default image viewer
Requires GraphViz (Windows: https://graphviz.gitlab.io/_pages/Download/Download_windows.html)

Usage: {} <filespec>
	""".format(os.path.basename(sys.argv[0]))
	
def GetScriptPath():
    return os.path.dirname(os.path.realpath(sys.argv[0]))
	
def ExecCommand(command):
	print('[Exec] ' + command)
	result = os.system(command)
	if result != 0:
		raise Exception("Command failed!")

def main(argv = None):
	if argv is None:
		argv = sys.argv
	
	if len(argv) < 2:
		PrintUsage()
		return 0

	filespec = argv[1]
		
	# Write dot file
	dotFile = os.path.join(tempfile.gettempdir(), os.path.basename(filespec) + '.dot')
	ExecCommand('"{}" {}'.format(sys.executable, os.path.join(GetScriptPath(), 'hsmToDot.py') + ' ' + filespec + ' > ' + dotFile))
	
	# Invoke dot to produce image
	pngFile = dotFile + '.png'
	ExecCommand('dot ' + dotFile + ' -Tpng -o' + pngFile)
	
	# Open default image viewer
	ExecCommand(pngFile)

if __name__ == "__main__":
	sys.exit(main())
