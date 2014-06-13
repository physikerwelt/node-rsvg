#!/usr/bin/env python

import os
import re
import sys
from subprocess import Popen, PIPE

def main():
	GTK_Root = sys.argv[1]
	packages = ' '.join(sys.argv[2:])

	if not os.path.exists(GTK_Root):
		print >> sys.stderr, "You must download GTK bundle for install node-rsvg in windows. More information at https://github.com/walling/node-rsvg#windows"
		return 1

	p = Popen(os.path.join(GTK_Root, 'bin/pkg-config') + ' --cflags-only-I %s' % packages, shell=True, stdout=PIPE)
	dirs = p.stdout.read()

	print re.sub(r'-I', '', dirs)

	return p.wait()


if __name__ == '__main__':
	sys.exit(main())
