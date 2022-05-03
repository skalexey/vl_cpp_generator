import os
import subprocess
import sys

args = ""
if (len(sys.argv) > 1):
	for i in range(len(sys.argv)):
		if i > 0:
			args = args + " " + sys.argv[i]

status = 0

if os.name == 'nt':
    status = os.system("build.bat" + args)
else:
    status = os.system("./build.sh . " + args)

exit(status)

