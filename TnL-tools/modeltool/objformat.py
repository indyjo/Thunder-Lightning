import sys
import vector
import operator
import string
import os
from math import *

def obj_import(filename):
    # scene is a tuple (mtllib or None, array of objects)
    # an object is a tuple (name, [v], [vn], [vt], [groups])
    # v,vn,vt are vectors
    # a group is a tuple (name, usemtl or None, [faces])
    # a face is an array [corners]
    # a corner is a tuple (v_idx, vt_idx or None, vn_idx or None)
    # all indices are 1-based!
    scene = (None, [])
    objects = scene[1]
    current_obj = None
    current_grp = None
    file = open(filename)
    for line in file:
        line = line.strip()
        if line.startswith("#"):
            continue
        words = string.split(line)
        if not words:
            continue
        if words[0] == "o":
            current_obj = (words[1], [], [], [], [])
            objname, v, vn, vt, groups = current_obj
            objects.append(current_obj)
            current_grp = None
        if words[0] == "g":
            current_grp = (words[1], None, [])
            grpname, usemtl, faces = current_grp
            groups.append(current_grp)
        elif words[0] == "v":
            x = string.atof(words[1])
            y = string.atof(words[2])
            z = string.atof(words[3])
            v.append(vector.Vector(x, y, z))
        elif words[0] == "vn":
            i = string.atof(words[1])
            j = string.atof(words[2])
            k = string.atof(words[3])
            vn.append(vector.Vector(i, j, k))
        elif words[0] == "vt":
            i = string.atof(words[1])
            j = string.atof(words[2])
            vt.append(vector.Vector(i, j))
        elif words[0] == "f":
            current_face = []
            for word in words[1:]:
                vtn = string.split(word, "/")
                corner = (string.atoi(vtn[0]), None, None)
                if len(vtn) > 1 and vtn[1]:
                    corner = (corner[0],string.atoi(vtn[1]),None)
                if len(vtn) > 2 and vtn[2]:
                    corner = (corner[0],corner[1],string.atoi(vtn[2]))

                current_face.append(corner)
            faces.append(current_face)
        elif words[0] == "mtllib":
            scene = (words[1], objects)
        elif words[0] == "usemtl":
            current_grp = (current_grp[0], words[1], current_grp[2])
            grpname, usemtl, faces = current_grp
            groups.pop()
            groups.append(current_grp)
    file.close()
    return scene
    
def obj_export(scene, filename):
    file = open(filename, "w")
    mtllib, objects = scene
    if mtllib is not None:
        file.write("mtllib "+mtllib+"\n")
    for obj in objects:
        objname, vlist, vnlist, vtlist, groups = obj
        if objname is not None:
            file.write("o "+objname+"\n")
        else:
            file.write("o\n")
        for v in vlist:
            file.write("v %f %f %f\n" % (v[0],v[1],v[2]))
        for v in vtlist:
            file.write("vt %f %f\n" % (v[0],v[1]))
        for v in vnlist:
            file.write("vn %f %f %f\n" % (v[0],v[1],v[2]))
        for grp in groups:
            grpname, usemtl, faces = grp
            if grpname is not None:
                file.write("g %s\n" % grpname)
            else:
                file.write("g\n")
            if usemtl is not None:
                file.write("usemtl %s\n" % usemtl)
            for face in faces:
                file.write("f")
                for corner in face:
                    v, vt, vn = corner
                    file.write(" %d" % v)
                    if vt is not None or vn is not None:
                        file.write("/")
                    if vt is not None:
                        file.write("%d" % vt)
                    if vn is not None:
                        file.write("/%d" % vn)
                file.write("\n")
    file.close()
    
