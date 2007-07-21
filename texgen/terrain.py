#!/usr/bin/python

import struct
import math
from tgafile import *
from vector import *
import perlin

class TerrainError:
    def __init__(self, text):
        self.text=text
    def __str__(self):
        return self.text
        
class TerrainDataError(TerrainError):
    pass
    
class TerrainCoordError(TerrainError):
    pass

class Terrain:
    def __init__(self, file, y_scale):
        t=TGAFileReader(file)
        t.readHeader()
        
        if t.getImageType() != TYPE_GREYSCALE:
            raise TerrainError("Image must be of type greyscale")
        
        t.readFile()
        self.width   = t.getWidth()
        self.length  = t.getHeight()
        self.data    = t.getImage()

        self.y_scale  = y_scale
        self.x_length = 1.0 / (self.width  - 1)
        self.z_length = 1.0 / (self.length - 1)
        
        self.params = {}
        self.cached = {}
        self.funcs = {
            "height":     self.__calcParamHeight,
            "height_int": self.__calcParamHeight,
            "angle":      self.__calcParamAngle,
            "fract":      self.__calcParamFract
        }
        
    def selectPixel(self, x, z):
        if x >= self.width:
            raise TerrainCoordError("x is too big")
        if x < 0:
            raise TerrainCoordError("x is too small")
        if z >= self.length:
            raise TerrainCoordError("z is too big")
        if z < 0:
            raise TerrainCoordError("z is too small")
            
        self.x=x
        self.z=z
        
        self.cached = {}
        #print "selectPixel (" + `x` + "/" + `z` + "): " + `self.params`
        
    def __getHeightAt(self, x, z):
        offset = (self.length - 1 - z) * self.length + x
        return struct.unpack("<B", self.data[offset])[0]
        
    def __calcParamHeight(self):
        self.params['height_int']=max(
            self.__getHeightAt(self.x,     self.z    ),
            self.__getHeightAt(self.x + 1, self.z    ),
            self.__getHeightAt(self.x + 1, self.z + 1),
            self.__getHeightAt(self.x,     self.z + 1))
        
        self.params['height']=float(self.params['height_int']) / 255.0
        self.cached['height']=1
        self.cached['height_int']=1
    
    def __calcParamAngle(self):
        v00 = [0.0,           float(self.__getHeightAt(self.x,     self.z + 1)) * self.y_scale, self.z_length]
        v10 = [self.x_length, float(self.__getHeightAt(self.x + 1, self.z + 1)) * self.y_scale, self.z_length]
        v01 = [0.0,           float(self.__getHeightAt(self.x,     self.z    )) * self.y_scale, 0.0]
        v11 = [self.x_length, float(self.__getHeightAt(self.x + 1, self.z    )) * self.y_scale, 0.0]
        
        #print v00, v10, v01, v11
        a=vsub(v11, v01)
        b=vsub(v00, v01)
        n=vcross(a,b)
        #print n
        angle1=math.acos(vscalar(n,[0.0, 1.0, 0.0]) / vlen(n)) * 180 / math.pi

        a=vsub(v00, v10)
        b=vsub(v11, v10)
        n=vcross(a,b)
        angle2=math.acos(vscalar(n,[0.0, 1.0, 0.0]) / vlen(n)) * 180 / math.pi

        self.params['angle']=max(angle1, angle2)
        self.cached['angle']=1
        
    def setFractParams(self, iterations, skip, persistence, xofs, zofs):
        self.iterations = iterations
        self.skip = skip
        self.persistence = persistence
        self.xofs = xofs
        self.zofs = zofs
        self.cached['fract']=0
        
    def __calcParamFract(self):
        u = float(self.x) / self.width
        v = float(self.z) / self.length

        weight = 0.5
        sum = 0.5
        for i in range(1,self.skip):
            u = u * 2.0
            v = v * 2.0
            weight = weight * self.persistence
            
        for i in range(1,self.iterations):
            sum = sum + weight * \
                perlin.noise2( u + self.xofs, v + self.zofs)
            u = u * 2.0
            v = v * 2.0
            weight = weight * self.persistence
        
        #print "fract: %f at %f %f" % (self.params['fract'], u, v)
        if sum > 1.0:
            sum = 1.0
        if sum < 0.0:
            sum = 0.0
        self.params['fract']=sum
        self.cached['fract']=1
        
    def getParam(self, pname):
        if not (self.cached.has_key('pname') and self.cached['pname']):
            self.funcs[pname]()
        return self.params[pname]
    
    def getWidth(self):
        return self.width
    
    def getLength(self):
        return self.length




