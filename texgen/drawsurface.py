#!/usr/bin/python

from tgafile import *
import array

class DrawingSurface:
    def __init__(self, file, width, height):
        self.file=file
        self.width=width
        self.height=height
        self.cmap=[]
        self.cindex=0
        
        self.image=[0] * self.width * self.height
    
    def setColorIndex(self, index, red, green, blue):
        self.cmap[index*3:index*3+2]=[red, green, blue]
    
    def selectColor(self, index):
        self.cindex=index
    
    def paintPixel(self, x, y):
        #self.image_data[y*self.width + x] = self.packed_color
        #self.image_data = self.image_data[0:y*self.width+x-1] + \
        #                  self.packed_color +                   \
        #                  self.image_data[y*self.width+x+1:]
        self.image[y*self.width+x] = self.cindex
        
    def save(self):
        image_data=array.array("B",self.image).tostring()
            
        t=TGAFileWriter(self.file, self.width, self.height, \
            TYPE_INDEXED, 24, image_data, 0, self.cmap)
        t.writeFile()
        
