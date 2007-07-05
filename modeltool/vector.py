import operator, types, math

class Vector:
    def __init__(self, *args):
	if type(args[0]) is types.InstanceType:
	    self.v = copy(args[0].v)
	elif (type(args[0]) is types.TupleType or
	      type(args[0]) is types.ListType):
	    self.v = map(float, args[0])
	else:
	    self.v = map(float, args)
	    
    def __str__(self):
	return "Vector" + str(self.v)

    def __repr__(self):
	return "Vector(" + repr(self.v) + ")"

    def __iadd__(self, v):
	self.v = map (operator.add, self.v, v.v)
	return self
	
    def __add__(self, v):
	new_v = Vector(self.v)
	new_v += v
	return new_v
	
    def __isub__(self, v):
	self.v = map (operator.sub, self.v, v.v)
	return self
	
    def __sub__(self, v):
	new_v = Vector(self.v)
	new_v -= v
	return new_v

    def __neg__(self):
	return Vector(map(operator.neg, self.v))
    
    def __pos__(self):
	return self
    
    def __mul__(self, other):
	if (type(other) is types.FloatType or
	    type(other) is types.IntType or
	    type(other) is types.LongType):
	    # Product with a scalar value
	    return Vector(
		map(lambda x: x*other, self.v))
	else:
	    # Dot product
	    return reduce( operator.add,
			   map( operator.mul,
			        self.v, other.v))

    def __rmul__(self, other):
	return other * self
	
    def __imul__(self, s):
	self.v = map(lambda x: x*s, self.v)
	return self
	
    def __idiv__(self, s):
	self *= 1.0 / s
	return self

    def __div__(self, s):
	return self * (1.0 / s)

    def __mod__(self, other):
	a = self.v
	b = other.v
	return Vector( ( a[1]*b[2] - a[2]*b[1],
			 a[2]*b[0] - a[0]*b[2],
			 a[0]*b[1] - a[1]*b[0] ))

    def __imod__(self, other):
	a = self.v
	b = other.v
	self.v = ( a[1]*b[2] - a[2]*b[1],
		   a[2]*b[0] - a[0]*b[2],
		   a[0]*b[1] - a[1]*b[0] )
	return self
    
    def __len__(self):
	return len(self.v)
    
    def __getitem__(self, i):
	return self.v[i]
	
    def __setitem__(self, i, v):
	self.v[i] = float(v)
	
    def __getslice__(self,i,j):
	return self.v[i:j]
    
    def __setslice__(self,i,j,seq):
	self.v[i:j] = map(float, seq)
    
    def length(self):
	return math.sqrt(self*self)
    
    def lengthSquared(self):
	return self*self
    
    def normalize(self):
	self /= self.length()
	
    def normalized(self):
	new_v = Vector(self)
	new_v.normalize()
	return new_v
	
    def projectedOn(self,other):
	return other * ((self * other) / (other*other))
	
    