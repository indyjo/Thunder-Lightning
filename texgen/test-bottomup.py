#!/usr/bin/python
from tgafile import *
import sys

f=open(sys.argv[1])
t=TGAFileReader(f)
t.readHeader()
if t.isBottomUp():
	print sys.argv[1] + " is BottomUp"
else:
	print sys.argv[1] + " is TopDown"

