import sys
import vector
import operator
import string
import os
from math import *
import wildcard

class Assignment:
    def __init__(self, objstring, grpstring, val):
        self.obj = wildcard.compile(objstring)
        self.grp = wildcard.compile(grpstring)
        self.val = val
    
    def matches(self, objname, grpname):
        return bool(self.obj.match(objname) and self.grp.match(grpname))

class AssignmentList:
    def __init__(self, assignment_list):
        self.assignments = assignment_list
        
    def valof(self, obj, grp):
        for a in self.assignments:
            if a.matches(obj, grp):
                return a.val
        return 0    


def clip(polygon, plane):
    vertices, transform, domain = polygon
    if len(vertices) == 0:
	return (vertices, transform, domain)
	
    (n,c) = plane
    result = []
    last = vertices[-1]
    last_d = n*last + c
    for current in vertices:
	current_d = n*current + c
	if current_d * last_d < 0.0:
	    new_point = (last * abs(current_d) +
			 current * abs(last_d))
	    new_point /= abs(last_d) + abs(current_d)
	    result += [new_point]
	if (current_d >= 0.0):
	    result += [current]
	last = current
	last_d = current_d
    return (result, transform, domain)
	
def bounds(vertices):
    n = len(vertices[0])
    result = []
    for i in range(n):
	values = map(lambda v: v[i], vertices)
	minimum = reduce(min, values)
	maximum = reduce(max, values)
	result += [(minimum, maximum)]
    return result


class IntervalEdge:
    def __init__(self, type, value):
	self.type = type
	self.value = value
	self.left = 0
	self.right = 0
	self.count = 0
    
    def is_lower(self):
	return self.type == 0

    def is_upper(self):
	return self.type == 1
	
    def compare(a,b):
	if a.value == b.value:
	    return a.type - b.type
	if a.value < b.value:
	    return -1
	return 1
	
    def rating(a,b):
	return 2*abs(a.left - b.right) + a.count


# calculates minimum and maximum signed distance of a poly to a plane
def minmax(poly, plane):
    (n,c) = plane
    if len(poly) == 0:
	return (0.0, 0.0)
    
    min = max = n * poly[0] + c
    for vertice in poly:
	d = n * vertice + c
	if d < min:
	    min = d
	if d > max:
	    max = d
    return (min, max)

def make_list(polys, plane):
    edges = []
    for poly in polys:
	if len(poly) == 0:
	    continue
	(min, max) = minmax(poly, plane)
	edges.append(IntervalEdge(0, min))
	edges.append(IntervalEdge(1, max))
    edges.sort(IntervalEdge.compare)
    
    before = 0
    for edge in edges:
	edge.left = before
	if edge.is_lower():
	    before += 1
    
    edges.reverse()
    
    before = 0
    for edge in edges:
	edge.right = before
	if edge.is_upper():
	    before += 1
    
    edges.reverse()
    
    count = 0
    for edge in edges:
	if edge.is_lower():
	    count += 1
	else:
	    count -= 1
	edge.count = count
	
    new_edges = []
    lower_edges = []
    for edge in edges:
	if edge.is_lower():
	    lower_edges.append(edge)
	else:
	    if (len(lower_edges) > 0 and
		abs(lower_edges[-1].value - edge.value) < 1e-5):
		lower_edges.pop()
	    else:
		new_edges.extend(lower_edges)
		new_edges.append(edge)
		lower_edges = []
    if len(lower_edges) != 0:
	print "Errorrrrrrrrrr!!!!!!!!!!!!!!"
    edges = new_edges
	
    new_edges = []
    for edge in edges:
	if len(new_edges) == 0:
	    new_edges.append(edge)
	    continue
	last_edge = new_edges[-1]
	if (last_edge.type == edge.type and
	    last_edge.value == edge.value):
	    if edge.is_lower():
		new_edges[-1].count = edge.count
		new_edges[-1].right = edge.right
	    else:
		new_edges[-1].count = edge.count
		new_edges[-1].left = edge.left
	else:
	    new_edges.append(edge)
    edges = new_edges
    
    return edges
	
    print "Edges generated:"
    if len(edges) == 0:
	print "None!"
	return edges
    
    last_edge = edges[0]
    for edge in edges:
	if edge.type == 0:
	    t = "lower"
	else:
	    t = "upper"
	print "%s at %f (equal=%d), left=%d right=%d count=%d rating = %d" % (
	    t, edge.value, edge.value==last_edge.value,
	    edge.left, edge.right, edge.count,
	    IntervalEdge.rating(last_edge, edge))
	last_edge = edge
    return edges

def find_best(edges):
    if len(edges) == 0:
	return -1, 0.0
	
    for i in xrange(len(edges) - 1):
	rating = IntervalEdge.rating(edges[i], edges[i+1])
	if i == 0 or rating < min_rating:
	    min_rating = rating
	    min_value = 0.5*(edges[i].value+edges[i+1].value)
    
    return min_rating, min_value
	

def make_bounds(eps, polys):
    #polys = filter(len, polys) # sort out empty polys
    vertices = []
    for vs, t, d in polys:
        vertices.add( vs )
    if len(vertices) == 0:
	return None
    bnds = bounds(vertices)
    
    for i in range(len(bnds)):
	a,b = bnds[i]
	d = b-a
	if i == 0 or d > longest:
	    longest = d
	    longest_axis = i
    if longest <= eps:
	return bnds, None, None, polys

    n = vector.Vector([0.0] * len(bnds))
    n[longest_axis] = 1.0
    c = - 0.5*(bnds[longest_axis][0]+bnds[longest_axis][1])
    return (bnds, 
	make_bounds(eps,
	    map(lambda poly: clip(poly, (n,c)), polys)),
	make_bounds(eps,
	    map(lambda poly: clip(poly, (-n,-c)), polys)),
	[])

def get_transforms(polys):
    transforms={}
    for poly in polys:
        transforms[poly[1]] = 1
    return transforms

def count_domains(polys):
    domains={}
    for poly in polys:
        domains[poly[2]] = 1
    return len(domains)

def split_polys_by_transform(polys, transform):
    polys1=[]
    polys2=[]
    for poly in polys:
        if poly[1]==transform:
            polys1.append(poly)
        else:
            polys2.append(poly)
    return polys1, polys2

def split_polys_by_domain(polys, domain):
    polys1=[]
    polys2=[]
    for poly in polys:
        if poly[2]==domain:
            polys1.append(poly)
        else:
            polys2.append(poly)
    return polys1, polys2

INNER=1
LEAF=2
NEWDOMAIN=3
NEWTRANSFORM=4
GATE=5

# Flag to define whether the bounding hierarchy should be divided on the root
# level into trees with a unique transform id.
# Setting this to False is backward compatible but will in most cases result
# in incorrect bounding hierarchies.
USE_GATE_FOR_XFORMS=True

def make_tree(eps, cur_transform, cur_domain, polys):
    # filter out polys without vertices
    polys = filter(lambda poly: len(poly[0]) > 0, polys)

    vertices = []
    for (vs, t, d) in polys:
        vertices.extend( vs )
    if len(vertices) == 0:
	return None
    bnds = bounds(vertices)

    contained_xforms = get_transforms(polys)
    n_transforms = len(contained_xforms)
    n_domains = count_domains(polys)
    print "n_transforms",n_transforms,contained_xforms
    
    if USE_GATE_FOR_XFORMS and n_transforms > 1:
        children = []
        for xform_id in contained_xforms.keys():
            polys_with_xform_id, polys = split_polys_by_transform(polys, xform_id)
            children.append(
                make_tree(eps, cur_transform, cur_domain, polys_with_xform_id))
        return (GATE, children)
    
    if n_transforms == 1 and polys[0][1] != cur_transform:
        return (NEWTRANSFORM, polys[0][1], bnds,
            make_tree(eps, polys[0][1], cur_domain, polys))
    if n_domains == 1 and polys[0][2] != cur_domain:
        return (NEWDOMAIN, polys[0][2], bnds,
            make_tree(eps, cur_transform, polys[0][2], polys))
    
    if n_transforms > 1:
        new_transform = polys[0][1]
        polys1, polys2 = split_polys_by_transform(polys, new_transform)
        return (INNER, bnds, 
                (NEWTRANSFORM, new_transform, bnds,
                    make_tree(eps, new_transform, cur_domain, polys1)),
                make_tree(eps, cur_transform, cur_domain, polys2))
    
    # Find out the longest bounding box axis
    # If it is shorter than eps, we are done
    for i in range(len(bnds)):
	a,b = bnds[i]
	d = b-a
	if i == 0 or d > longest:
	    longest = d
	    longest_axis = i
	#maxpolys=32
    if longest <= eps: # and len(polys) <= maxpolys:
        return (LEAF, bnds, polys)

        
    
    if n_domains > 1:
        new_domain = polys[0][2]
        polys1, polys2 = split_polys_by_domain(polys, new_domain)
        return (INNER, bnds,
            (NEWDOMAIN, new_domain, bnds, make_tree(eps, cur_transform, new_domain, polys1)),
            make_tree(eps, cur_transform, cur_domain, polys2))
            
    # print "longest axis is %d with %.2f, still bigger than %f with %d polys" %(longest_axis, longest, eps, len(polys))
    # subdivide!
    # TODO: find better clipping plane
    n = vector.Vector(0,0,0)
    n[longest_axis] = 1.0
    c = - 0.5*(bnds[longest_axis][0]+bnds[longest_axis][1])
    return (INNER, bnds,
        make_tree(eps, cur_transform, cur_domain,
            map(lambda poly: clip(poly, (n,c)), polys)),
        make_tree(eps, cur_transform, cur_domain,
            map(lambda poly: clip(poly, (-n,-c)), polys)))
    

nodetypes={
    INNER:"INNER",
    LEAF:"LEAF",
    NEWDOMAIN:"NEWDOMAIN",
    NEWTRANSFORM:"NEWTRANSFORM",
    GATE:"GATE"
}

def prettyprint(tree, indent=0):
    if tree[0] == INNER:
        nodetype, bnds, sub1, sub2 = tree
        print "  "*indent+"INNER (% 6.2f, % 6.2f) (% 6.2f, % 6.2f) (% 6.2f, % 6.2f)" %(
            bnds[0][0], bnds[0][1], bnds[1][0], bnds[1][1], bnds[2][0], bnds[2][1])
        prettyprint(sub1, indent + 1)
        prettyprint(sub2, indent + 1)
    elif tree[0] == NEWDOMAIN or tree[0] == NEWTRANSFORM:
        nodetype, newval, bnds, child = tree
        print "  "*indent+"%s -> %d" %(nodetypes[nodetype], newval)
        prettyprint(child, indent + 1)
    elif tree[0] == GATE:
        nodetype, children = tree
        print "  "*indent+"GATE (%d children)" % len(children)
        for c in children:
            prettyprint(c, indent+1)
    else:
        nodetype, bnds, polys = tree
        print "  "*indent+"LEAF (% 6.2f, % 6.2f) (% 6.2f, % 6.2f) (% 6.2f, % 6.2f) polys: %d" %(
            bnds[0][0], bnds[0][1], bnds[1][0], bnds[1][1], bnds[2][0], bnds[2][1], len(polys))

        
    

# Entfernt Kindknoten, die weniger als eps Volumeneinheiten
# Einsparung bringen
def compress_bounds(eps, node):
    print "Sorry, feature disabled."
    return
    if node is None:
	return (None, 0.0, 0.0)
    b,t1,t2,polys = node
    vol = reduce(operator.mul,
        map(lambda x: x[1]-x[0], b))
    if t1 is None or t2 is None or vol==0.0:
	return (node, vol, 0.0)
    preserved = vol
    child1, child_vol, child_preserved = compress_bounds(
	eps, t1)
    if child1 is not None:
	preserved += child_preserved
	preserved -= child_vol
    child2, child_vol, child_preserved = compress_bounds(
	eps, t2)
    if child2 is not None:
	preserved += child_preserved
	preserved -= child_vol
    if (preserved/vol < eps):
	b1,t11,t12,polys1 = child1
	b2,t21,t22,polys2 = child2
	polys1.extend(polys2)
	return ((b, None, None, polys1), vol, preserved)
    else:
	return ((b, child1, child2, polys), vol, preserved)
    

def print_bounds(indent, node):
    if node is None:
	print " "*indent+"None"
	return
    b,t1,t2,polys = node
    print (" "*indent)+"( "+str(b)
    print_bounds(indent+1, t1)
    print_bounds(indent+1, t2)
    print (" "*indent)+")"

def max_polys_in(node):
    if node is None: return 0
    b,t1,t2,polys = node
    return max(len(polys), max_polys_in(t1), max_polys_in(t2))

# returns a list of tuples ( vertices, transform, domain ) where vertices is a list of vectors
def obj_import(filename, transforms, domains):
    vcount  = 0
    vncount = 0
    fcount  = 0
    pointList    = []
    normalList   = []
    faceList     = []
    materialList = []
    file = open(filename, "r")
    lines = file.readlines()
    linenumber = 0
    current_obj = "default"
    current_grp = "default"
    for line in lines:
        line = line.strip()
        linenumber = linenumber + 1
        if line.startswith("#"):
            continue
        words = string.split(line)
        if not words:
            continue
        if words[0] == "o":
            current_obj = words[1]
        if words[0] == "g":
            current_grp = words[1]
        elif words[0] == "v":
            vcount = vcount + 1
            x = string.atof(words[1])
            y = string.atof(words[2])
            z = string.atof(words[3])
            pointList.append(vector.Vector(x, y, z))

        elif words[0] == "vn":
            vncount = vncount + 1
            i = string.atof(words[1])
            j = string.atof(words[2])
            k = string.atof(words[3])
            normalList.append(vector.Vector(i, j, k))
        elif words[0] == "f":
            fcount = fcount + 1
            vi = [] # vertex  indices
            ti = [] # texture indices
            ni = [] # normal  indices
            words = words[1:]
            lcount = len(words)
            for index in (xrange(lcount)):
                vtn = string.split(words[index], "/")
                vi.append(string.atoi(vtn[0]))
                if len(vtn) > 1 and vtn[1]:
                    ti.append(string.atoi(vtn[1]))
                if len(vtn) > 2 and vtn[2]:
                    ni.append(string.atoi(vtn[2]))
            
            transform = transforms.valof(current_obj, current_grp)
            domain = domains.valof(current_obj, current_grp)
            faceList.append( (vi, ti, ni, transform, domain) )
        elif words[0] == "mtllib":
            continue
	    # try to export materials
            directory, dummy = os.path.split(filename)
            filename = os.path.join(directory, words[1])
            try:
                file = open(filename, "r")
            except:
                print "no material file %s" % filename
            else:
                file = open(filename, "r")
                line = file.readline()
                while line:
                    words = string.split(line)
                    if words and words[0] == "newmtl":
                        name = words[1]
                        file.readline() # Ns .
                        file.readline() # d .
                        file.readline() # illum .
                        line = file.readline()
                        words = string.split(line)
                        Kd = [string.atof(words[1]),
                              string.atof(words[2]),
                              string.atof(words[3])]
                        line = file.readline()
                        words = string.split(line)
                        Ks = [string.atof(words[1]),
                              string.atof(words[2]),
                              string.atof(words[3])]
                        line = file.readline()
                        words = string.split(line)
                        Ka = [string.atof(words[1]),
                              string.atof(words[2]),
                              string.atof(words[3])]
                        materialList.append([name, Kd, Ks, Ka])
                    line = file.readline()
                file.close()
        elif words[0] == "usemtl":
            name = words[1]
            #print "usemtl %s" % name
    file.close()
    
    polys = []
    for face in faceList:
        vertices = map(lambda i: pointList[i-1], face[0])
        # vertices, transform, domain
        polys.append( (vertices, face[-2], face[-1]) )
    return polys



    
    
