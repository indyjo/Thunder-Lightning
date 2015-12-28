#!/usr/bin/python

from poly import *
from copy import copy
from math import sqrt

class ExportAsObj:
    def __init__(self, f):
        self.f = f
        self.bcount = 0
        self.vcount = 0
        
    def export(self, node):
        nodetype = node[0]
        if nodetype == INNER:
            self.export(node[2])
            self.export(node[3])
            return
        elif nodetype == NEWDOMAIN or nodetype == NEWTRANSFORM:
            self.export(node[3])
            return
        elif nodetype == GATE:
            _t, children = node
            for c in children:
                self.export(c)
            return
        
        b = node[1]

    

        x_min, x_max = b[0]

        y_min, y_max = b[1]

        z_min, z_max = b[2]

    

        print >> self.f, "g"

        print >> self.f, "v ", x_min, y_min, z_min

        print >> self.f, "v ", x_min, y_min, z_max

        print >> self.f, "v ", x_min, y_max, z_min

        print >> self.f, "v ", x_min, y_max, z_max

        print >> self.f, "v ", x_max, y_min, z_min

        print >> self.f, "v ", x_max, y_min, z_max

        print >> self.f, "v ", x_max, y_max, z_min

        print >> self.f, "v ", x_max, y_max, z_max

    

        print >> self.f, "g group%d" % (self.bcount)

        print >> self.f, "f %d %d %d %d" % (1 + self.vcount, 2 + self.vcount, 4 + self.vcount, 3 + self.vcount)

        print >> self.f, "f %d %d %d %d" % (2 + self.vcount, 6 + self.vcount, 8 + self.vcount, 4 + self.vcount)

        print >> self.f, "f %d %d %d %d" % (6 + self.vcount, 5 + self.vcount, 7 + self.vcount, 8 + self.vcount)

        print >> self.f, "f %d %d %d %d" % (5 + self.vcount, 1 + self.vcount, 3 + self.vcount, 7 + self.vcount)

        print >> self.f, "f %d %d %d %d" % (3 + self.vcount, 4 + self.vcount, 8 + self.vcount, 7 + self.vcount)

        print >> self.f, "f %d %d %d %d" % (5 + self.vcount, 6 + self.vcount, 2 + self.vcount, 1 + self.vcount)

    

        self.vcount += 8
        self.bcount += 1



class ExportAsBounds:
    def triangulate(self, polys):

        new_polys = []

        for (verts, t, d) in polys:
            while len(verts) > 3:

    	       new_polys.append( ([verts[-2], verts[-1], verts[0]], t, d) )

    	       verts.pop()
    	    if len(verts)<3:

    	       continue
    	    if len(verts) == 3:
    	       new_polys.append((verts,t,d))
        return new_polys

    		

    

    def bounding_radius(self, node):

        if node[0]==INNER:
            return max(self.bounding_radius(node[2]), self.bounding_radius(node[3]))
        elif node[0]==NEWDOMAIN or node[0]==NEWTRANSFORM:
            return self.bounding_radius(node[3])
        elif node[0]==GATE:
            children = node[1]
            return max( map(self.bounding_radius, children) )
        else:
            return max(map(lambda poly: max([0]+map(lambda v: sqrt(v*v), poly[0])), node[2]))
            
    def export_box(self, bnds):

        self.append("Box( Vector3( %f %f %f ) %f %f %f )" % (

            0.5*(bnds[0][0]+bnds[0][1]),
            0.5*(bnds[1][0]+bnds[1][1]),
            0.5*(bnds[2][0]+bnds[2][1]),
            0.5*(bnds[0][1]-bnds[0][0]),
            0.5*(bnds[1][1]-bnds[1][0]),
            0.5*(bnds[2][1]-bnds[2][0])))

    

    def export_node(self, node):
        nodetype = node[0]
        if nodetype == INNER:
            nodetype, bnds, sub1, sub2 = node
            self.write("Node( ");
            self.export_box(bnds)
            self.append(" I\n")
            self.indent += 1
            self.export_node(sub1)

            self.export_node(sub2)

            self.indent -= 1
            self.write(")\n")

        elif nodetype == LEAF:
            nodetype, bnds, polys = node
            self.write("Node( ");
            self.export_box(bnds)
            self.append(" L\n")
            polys = self.triangulate(polys)
            self.indent += 1
            self.write(len(polys),"\n")
            for poly in polys:
                for v in poly[0]:
                    self.write("Vector3( %f %f %f )\n" % (v[0],v[1],v[2]))
            self.indent -= 1
            self.write(")\n")

        elif nodetype == NEWTRANSFORM:
            nodetype, newval, bnds, child = node
            self.write("Node( ")
            self.export_box(bnds)
            self.append(" T ", newval, "\n")
            self.indent += 1
            self.export_node(child)
            self.indent -= 1
            self.write(")\n")

        elif nodetype == NEWDOMAIN:
            nodetype, newval, bnds, child = node
            self.write("Node( ")
            self.export_box(bnds)
            self.append(" D ", newval, "\n")
            self.indent += 1
            self.export_node(child)
            self.indent -= 1
            self.write(")\n")
        elif nodetype == GATE:
            nodetype, children = node
            self.write("Gate( %d\n" % len(children))
            self.indent += 1
            for c in children:
                self.export_node(c)
            self.indent -= 1
            self.write(")\n")
            

        

        

    def __init__(self, f, parents):
        self.f = f
        self.indent = 0
        self.parents = copy(parents)
        
    def write(self, *args):
        self.f.write(" "*(self.indent))
        for arg in args:
            self.f.write(str(arg))
            
    def append(self, *args):
        for arg in args:
            self.f.write(str(arg))
        
    def all_domains(self, node, domains={}):
        if node[0]==INNER:
            self.all_domains(node[2],domains)
            self.all_domains(node[3],domains)
        elif node[0]==NEWDOMAIN:
            domains[node[1]]=1
            self.all_domains(node[3],domains)
        elif node[0]==NEWTRANSFORM:
            self.all_domains(node[3],domains)
        elif node[0]==GATE:
             children = node[1]
             for c in children:
                self.all_domains(c, domains)
        return domains
        
    def max_transform(self, node):
        if node[0]==INNER:
            return max(self.max_transform(node[2]), self.max_transform(node[3]))
        elif node[0]==NEWDOMAIN:
            return self.max_transform(node[3])
        elif node[0]==NEWTRANSFORM:
            return max(node[1], self.max_transform(node[3]))
        elif node[0]==GATE:
             children = node[1]
             return max( map(self.max_transform, children) )
        else:
            return 0
        
    def export(self, node):
        self.write("Geometry( %f\n" % self.bounding_radius(node))
        self.write("(")
        for i in self.all_domains(node).keys():
            self.append(" "+str(i))
        self.append(" ) \n")
        self.write("(")
        for i in range(self.max_transform(node)+1):
            if self.parents.has_key(i):
                self.append(" "+str(self.parents[i]))
            else:
                self.append(" 0")
        self.append(" )\n")
        self.indent += 1

        self.export_node(node)

        self.indent -= 1

        self.write(")\n")

        
