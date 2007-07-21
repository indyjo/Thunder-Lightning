#!/usr/bin/python
import math

def vcross(a,b):
    return [
        a[1]*b[2]-b[1]*a[2],
        a[0]*b[2]-b[0]*a[2],
        a[0]*b[1]-b[0]*a[1]]
        
def vscalar(a,b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
    
def vadd(a,b):
    return [
        a[0]+b[0],
        a[1]+b[1],
        a[2]+b[2]]
        
def vsub(a,b):
    return [
        a[0]-b[0],
        a[1]-b[1],
        a[2]-b[2]]

def vmul(a,s):
    return [
        a[0] * s,
        a[1] * s,
        a[2] * s]

def vlen(a):
    return math.sqrt(
        a[0]*a[0] +
        a[1]*a[1] +
        a[2]*a[2])
