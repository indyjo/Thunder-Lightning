#!/usr/bin/python

import sys, os
import struct
import Image

def spr2pngfile(infile_name, outfile_name):
    infile = open(infile_name, "r")
    header = infile.read(14)
    print len(header)
    (w,h) = struct.unpack("xxxx H H xx xx xx", header)
    print infile_name, "is a %dx%d image" % (w,h)
    im = Image.fromstring("RGBA", (w,h), infile.read(4*w*h),
        "raw", "RGBA", 4*w)
    im.save(outfile_name)
    


def usage():
    print "Usage: "+sys.argv[0]+" <infile.spr> <outfile.png>"


if len(sys.argv)!=3:
    usage()
    sys.exit(-1)

spr2pngfile(sys.argv[1], sys.argv[2])
