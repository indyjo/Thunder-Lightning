#!/usr/bin/python

import vector
from copy import copy

# skeletonspec: [entries]
# entry: (childobj, parentobj, pivot)
# childobj: string
# parentobj: string or none
# pivot: vector

def read_spec(file):
    spec = []
    for line in file:
        line = line.strip()
        if line.startswith("#"):
            continue
        if len(line) == 0:
            continue
        tokens = line.split()
        if len(tokens) == 4:
            childobj, x, y, z = tokens
            parentobj = None
        else:
            parentobj, childobj, x, y, z = tokens
        
        spec.append( (childobj, parentobj, vector.Vector(x,y,z)) )
    return spec

# translatetable: {objname: translate}
# objname: string
# translate: vector

def make_translate_table(spec):
    spec = copy(spec)
    table = {}
    for entry in spec:
        childobj, parentobj, pivot = entry
        table[childobj] = -pivot
            
    return table
    
def reverse_table(table):
    for key in table:
        table[key] = -table[key]
        
