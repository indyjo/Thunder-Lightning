#!/usr/bin/python

import string
from drawsurface import *
from terrain import *


# exceptions -------------------------------------------

class XMLException:
    def __init__(self, text):
        self.text=text
    def __str__(self):
        return `self.text`

class XMLHierarchyException(XMLException):
    pass

class XMLNameException(XMLException):
    pass


# XML classes ------------------------------------------

# The Base class.
class XMLObject:
    def __init__(self, parent, args):
        self.parent=parent
        if self.parent is not None:
            self.parent._addChild(self)
    def _addChild(self, child):
        pass
    def getChildren(self):
        return []

# A class that can have zero or one child
# Throws XMLHierarchyException when added another child
class XMLBin(XMLObject):
    def __init__(self, parent, args):
        self.child=None
        XMLObject.__init__(self, parent, args)
    def _addChild(self, child):
        if self.child is not None:
            raise XMLHierarchyException("Can't add more than one child to XMLBin objects")
        print "XMLBin._addChild(" +  `child` +")"
        self.child=child
    def getChildren(self):
        if self.child is not None:
            return [self.child]
        else:
            return []

        
# A class that can have many children
class XMLContainer(XMLObject):
    def __init__(self, parent, args):
        self.children=[]
        XMLObject.__init__(self, parent, args)
    def _addChild(self, child):
        print "XMLContainer._addChild(" +  `child` +")"
        self.children.append(child)
    def getChildren(self):
        return self.children

# Now the concrete classes ------------------------

# The base class responsible for reading command line parameters and managing
# global settings
class Texgen(XMLContainer):
    def __init__(self, parent, args):
        XMLContainer.__init__(self, parent, args)
        
        #default values for args
        self.input='in.tga'
        self.output='out.tga'
        self.y_factor=1.0
        
        #set the args if avail
        if args.has_key('input'):
            self.input=args['input']
        if args.has_key('output'):
            self.output=args['output']
        if args.has_key('y_factor'):
            self.y_factor=float(args['y_factor'])

        self.infile=open(self.input)
        self.outfile=open(self.output,"w")

        self.terrain=Terrain(self.infile, self.y_factor)
        self.width=self.terrain.getWidth()
        self.length=self.terrain.getLength()
        self.surface=DrawingSurface(self.outfile, self.width - 1, self.length - 1)
    def _addChild(self, child):
        if not isinstance(child, Texture):
            raise XMLHierarchyException('Child of Texgen must be a texture')
        red, green, blue = child.getColor()
        print "added child with color " + `red` + ", " + `green` + ", " + `blue` + "."
        self.surface.setColorIndex(len(self.getChildren()), red, green, blue)
        XMLContainer._addChild(self, child)
        
    def run(self):
        children = self.getChildren()
        print "Texgen.run(): children:" + `children`
        
        for y in range(0, self.width-1):
            print `y` +" / " + `self.width - 1`
            for x in range(0, self.length-1):
                self.terrain.selectPixel(x,self.length - 2 - y)
                index = 0
                for child in children:
                    if child.run(self.terrain):
                        self.surface.selectColor(index)
                        self.surface.paintPixel(x,y)
                        break
                    index = index + 1
        
        self.surface.save()
        


# The class Texture stands for one layer of texture. It is a bin object,
# which means that it may only contain one child and that must be of type
# Operator
class Texture(XMLBin):
    def __init__(self, parent, args):
        #default values for args
        self.name  = "DefaultTextureName"
        self.fract = 0.5
        self.color = [255,0,255] # An ugly magenta
        
        if args.has_key('name'):
            self.name = args['name']
        if args.has_key('fract'):
            self.fract = float(args['fract'])
        if args.has_key('color'):
            self.color = []
            for colstr in string.split(args['color'],','):
                self.color.append(int(colstr))

        XMLBin.__init__(self, parent, args)
    def _addChild(self, child):
        if not isinstance(child, Operator):
            raise XMLHierarchyException('Child of Texture must be an operator')
        XMLBin._addChild(self, child)
    def run(self, terrain):
        return self.getChildren()[0].run(terrain)
    def getColor(self):
        return self.color


# This is the base class for the operators
# Operators can form a hierarchy. They can be called and return a boolean value
class Operator(XMLContainer):
    def __init__(self, parent, args):
        XMLContainer.__init__(self, parent, args)

    def _addChild(self, child):
        if not isinstance(child, Operator):
            raise XMLHierarchyException('Child of Operator must be an operator')
            
        XMLContainer._addChild(self, child)
    def run(self, terrain):
        return 1
    
class OpTrue(Operator):
    def _addChild(self, child):
        raise XMLHierarchyException('Operator True has no children.')
    def run(self, terrain):
        return 1

class OpFalse(Operator):
    def _addChild(self, child):
        raise XMLHierarchyException('Operator False has no children.')
    def run(self, terrain):
        raise XMLException('Virtual Base class called')

class OpAnd(Operator):
    def run(self, terrain):
        for child in self.getChildren():
            if not child.run(terrain):
                return 0
        return 1

class OpOr(Operator):
    def run(self, terrain):
        for child in self.getChildren():
            if child.run(terrain):
                return 1
        return 0

class OpXor(Operator):
    def _addChild(self, child):
        if len(self.getChildren()) == 2:
            raise XMLHierarchyException('Operator Xor has already 2 children.')
        XMLContainer._addChild(child)
        
    def run(self, terrain):
        if len(self.getChildren()) != 2:
            raise XMLHierarchyException('Operator Xor must have exactly 2 children.')
        for child in self.getChildren():
            if child.run(terrain):
                return 1
        return 0

class OpNot(Operator):
    def _addChild(self, child):
        if len(self.getChildren()) == 1:
            raise XMLHierarchyException('Operator Not has already a child.')
        XMLContainer._addChild(child)

    def run(self, terrain):
        if len(self.getChildren()) != 1:
            raise XMLHierarchyException('Operator Xor must have exactly one child.')

class OpRange(Operator):
    def __init__(self, parent, args):
        if (not args.has_key('min'))  or \
           (not args.has_key('max')):
            raise XMLNameException('Operator Range needs attributes min, max')
        Operator.__init__(self, parent, args)
        
        self.min=float(args['min'])
        self.max=float(args['max'])
        
    def _addChild(self, child):
        if len(self.getChildren()) == 1:
            raise XMLHierarchyException('Operator Range has already a child.')
        if not isinstance(child, DataSource):
            raise XMLHierarchyException('Operator Range takes a data source.')
        XMLContainer._addChild(self, child)
        #raise XMLHierarchyException('Operator Range has no children.')
        
    def run(self, terrain):
        value=self.children[0].run(terrain)
        return (value >= self.min) and (value <= self.max)

# Data sources return a float value
class DataSource:
    pass
    
class HeightSource(DataSource, XMLObject):
    def _addChild(self, child):
        raise XMLHierarchyException('Operator HeightSource has no children.')
    def run(self, terrain):
        return float(terrain.getParam("height"))
    
class HeightIntSource(DataSource, XMLObject):
    def _addChild(self, child):
        raise XMLHierarchyException('Operator HeightIntSource has no children.')
    def run(self, terrain):
        return float(terrain.getParam("height_int"))
    
class AngleSource(DataSource, XMLObject):
    def _addChild(self, child):
        raise XMLHierarchyException('Operator AngleSource has no children.')
    def run(self, terrain):
        return float(terrain.getParam("angle"))

class FractSource(DataSource, XMLObject):
    def __init__(self, parent, args):
        if args.has_key('iterations'):
            self.iterations = int(args['iterations'])
        else:
            self.iterations = 5
            
        if args.has_key('skip'):
            self.skip = int(args['skip'])
        else:
            self.skip = 1
            
        if args.has_key('persistence'):
            self.persistence = float(args['persistence'])
        else:
        	self.persistence = 0.75
        
        if args.has_key('xofs'):
            self.xofs = float(args['xofs'])
        else:
            self.xofs = 0.0

        if args.has_key('zofs'):
            self.zofs = float(args['zofs'])
        else:
            self.zofs = 0.0
            
        XMLObject.__init__(self, parent, args)
        
    def _addChild(self, child):
        raise XMLHierarchyException('Operator FractSource has no children.')
        
    def run(self, terrain):
        terrain.setFractParams(self.iterations, self.skip, \
            self.persistence, self.xofs, self.zofs)
        return float(terrain.getParam("fract"))
    

        
        
# In the xmlclasses dictionary we store the mapping between the xml tags and
# the python classes

xmlclasses = {
    'texgen'  : Texgen,
    'texture' : Texture,
    'true'    : OpTrue,
    'false'   : OpFalse,
    'and'     : OpAnd,
    'or'      : OpOr,
    'xor'     : OpXor,
    'not'     : OpNot,
    'range'   : OpRange,
    'height'  : HeightSource,
    'height_int' : HeightIntSource,
    'angle'   : AngleSource,
    'fract'   : FractSource,
}
