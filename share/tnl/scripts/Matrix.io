assert := method(v,
  if (v,
    self
  ,
    code := call argAt(0) code
    Exception raise("Assertion failed: " .. code)
  )
)

Matrix := Object clone do(
  rows := 0
  columns := 0
  entries := list()
  
  init := method(
    self entries := entries clone
  )

  at := method(r,c,
    c = c ifNilEval(0)
    entries at(c*rows+r)
  )
  atSet := method(r,c,v,
    if (v isNil, v=c; c=0);
    entries atPut(c*rows+r, v)
    self
  )

  dim := method(r,c,
    rows = r
    columns = c
    entries empty
    for(i,1,r*c, entries append (0))
    self
  )

  set := method(
    self entries empty
    args := call evalArgs
    for (c,0,columns-1, for (r,0,rows-1,
      entries append(args at (r*columns+c))
    ))
    self
  )

  setList := method(entries,
    self entries := entries
    self
  )
  
  transpose := method(
    M := Matrix clone dim(columns, rows)
    for (c,0,columns-1, for(r,0,rows-1,
      M atSet(c,r, self at(r,c))
    ))
    M
  )
  
  dot := method(other,
    assert(columns == 1)
    assert(other rows == rows and other columns == 1)
    
    val := 0
    entries foreach(i,v, val = val + v * other entries at(i))
    val
  )
  
  setSlot("'", getSlot("transpose") )

  setSlot("+", method(other,
    assert(other rows == rows and other columns == columns)
    M := Matrix clone dim(rows,columns)
    M entries foreach(i,v,
      M entries atPut(i, self entries at(i) + other entries at (i)))
    M
  ))

  setSlot("-", method(other,
    assert(other rows == rows and other columns == columns)
    M := Matrix clone dim(rows,columns)
    M entries foreach(i,v,
      M entries atPut(i, self entries at(i) - other entries at (i)))
    M
  ))

  setSlot("*", method(other,
    if(other hasProto(Matrix),
      matMult(other)
    ,
      M := Matrix clone dim(rows,columns)
      if(other hasProto(Number),
        self entries foreach(i,v, M entries atPut(i, v*other))
        M
      ,
        Exception raise("Type error","Invalid type " .. (other type) .. " for Matrix multiplication")
      )
    )
  ))

  matMult := method(other,
    assert(self columns == other rows)
    res := Matrix clone
    res rows := self rows
    res columns := other columns
    other columns repeat(j,
      self rows repeat(i,
        x := 0
        self_idx := i
        other_idx := j * other rows
        self columns repeat(
            x = x + self entries at(self_idx) * other entries at(other_idx)
            self_idx = self_idx + self rows
            other_idx = other_idx + 1
        )
        res entries append(x)
      )
    )
    res
  )
  
  scaleInPlace := method(s,
    self entries mapInPlace(v, v*s)
    self
  )
  scaledBy := method(s,
    self clone scaleInPlace(s)
  )
  
  projectedOn := method(other,
    other scaledBy( other dot(self) )
  )
  
  negInPlace := method(
    entries mapInPlace(v, -v)
    self
  )
  neg := method(
    self clone negInPlace
  )
  
  mixedWith := method(other, u,
    self + (other-self) scaleInPlace(u)
  )
  
  setSlot("%", method(other,
    assert(rows == 3 and columns == 1)
    assert(other hasProto(Matrix))
    assert(other rows == 3 and other columns == 1)
    v := Matrix clone dim(3,1)
    v set(at(1,0) * (other at(2,0)) - at(2,0) * (other at(1,0)),
          at(2,0) * (other at(0,0)) - at(0,0) * (other at(2,0)),
          at(0,0) * (other at(1,0)) - at(1,0) * (other at(0,0)))
  ))
  
  identity := method(n,
    M := Matrix clone dim(n,n)
    for(i,0,n-1, for(j,0,n-1, M atSet(i,j, if(i==j, 1, 0))))
    M
  )
  
  lenSquare := method(
    assert(columns==1)
    s := 0
    for(i,0,rows-1, s = s + entries at(i) * entries at(i))
    s
  )
  
  len := method(lenSquare sqrt)
  length := method(lenSquare sqrt)
  distTo := method(other, (other - self) len)
  
  norm := method(
    self scaledBy(1/self len)
  )
  normInPlace := method(
    self scaleInPlace(1/self len)
  )
  
  projection := method(v,
    assert(v columns == 1)
    v * v transpose
  )
  
  crossproduct := method(v,
    assert(v columns == 1 and v rows == 3)
    matrix(         0,  v at(2,0), -v at(1,0)
           -v at(2,0),          0,  v at(0,0)
            v at(1,0), -v at(0,0),        0)
  )
  
  rotation3 := method(axis, angle,
    S := crossproduct(axis)
    P := projection(axis)
    I := identity(3)
    return P + angle cos * (I-P) + angle sin * S
  )
  
  x := method( at(0) )
  y := method( at(1) )
  z := method( at(2) )
  w := method( at(3) )
  xy := method( vector( at(0), at(1) ) )
  xz := method( vector( at(0), at(2) ) )
  xw := method( vector( at(0), at(3) ) )
  yz := method( vector( at(1), at(2) ) )
  yw := method( vector( at(1), at(3) ) )
  zw := method( vector( at(2), at(3) ) )
  xyz := method( vector( at(0), at(1), at(2) ) )
  xyw := method( vector( at(0), at(1), at(3) ) )
  xzw := method( vector( at(0), at(2), at(3) ) )
  yzw := method( vector( at(1), at(2), at(3) ) )
  
  col := method(c,
    m := Matrix clone
    m rows := rows
    m columns := 1
    m entries := entries slice(c*rows, c*rows+rows)
    m
  )
    
  
  asString := method(
    if (columns == 1,
      res := "vector("
      entries foreach(i,v,
        if (i != 0 , res = res .. ", ")
        res = res .. v asString(2)
      )
      res .. ")"
    ,
      "matrix(...)"
    )
  )
  
  asSimpleString := method( asString )
      
)

vector := block(
  M := Matrix clone dim(call argCount, 1)
  values := call evalArgs
  Lobby vals := values
  //writeln("values: ", values)
  M setList( values )
) setIsActivatable(true)


Message do(
  isSemicolon := method( name == ";")
  hasSemicolonInChain := method(
    if (self isSemicolon, return true)
    msg := self
    while (msg = msg next,
      if (msg isSemicolon, return true)
    )
    return false
  )
  cutAtSemicolon := method(
    msg := self
    while( msg next,
      if (msg next isSemicolon,
        tail := msg next next
        msg setNext(nil)
        return tail
      )
      msg = msg next
    )
    nil
  )
)

matrix := method(
  rows := List clone append(List clone)
  maxcols := 0
  cols := 0
  rows := 0
  call message arguments foreach(arg,
    rows = rows max(1)
    cols = cols + 1
    maxcols = maxcols max(cols)
    msg := arg clone
    while( msg = msg cutAtSemicolon,
      cols = 1
      rows = rows + 1
    )
  )
  cols = maxcols
  
  mat := Matrix clone dim(rows, cols)
  msg := tail := nil
  i := j := 0
  call message arguments foreach(arg,
    msg = arg clone
    while (msg,
      tail = msg cutAtSemicolon
      mat atSet(i,j, msg doInContext(call sender) )
      if (tail isNil not,
        i = i + 1
        j = 0
      )
      msg = tail
    )
    j = j + 1
  )
  mat
)

