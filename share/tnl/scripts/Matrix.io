Lobby Matrix := NMatrix
NMatrix type = "NMatrix"

assert := method(v,
  if (v,
    self
  ,
    code := call argAt(0) code
    Exception raise("Assertion failed: " .. code)
  )
)

assert := nil

NMatrix do(
  setList := method( values,
    r:=0
    c:=0
    values foreach(v,
      self atSet(r,c,v)
      r = r + 1
      if (r == self rows,
        r = 0
        c = c+1
      )
    )
    self
  )
  
  transpose := method(
    self clone transposeInPlace
  )
  
  setSlot("'", getSlot("transpose") )

  setSlot("*", method(other,
    if(other hasProto(NMatrix),
      matMult(other)
    ,
      M := NMatrix clone dim(rows,columns)
      if(other hasProto(Number),
        self scaledBy(other)
      ,
        Exception raise("Type error","Invalid type " .. (other type) .. " for NMatrix multiplication")
      )
    )
  ))

  scaledBy := method(s,
    self clone scaleInPlace(s)
  )
  
  projectedOn := method(other,
    other scaledBy( other dot(self) )
  )
  
  negInPlace := method(
    scaleInPlace(-1)
  )
  neg := method(
    self clone negInPlace
  )
  
  mixedWith := method(other, u,
    self + (other-self) scaleInPlace(u)
  )
  
  setSlot("%", getSlot("cross"))
  
  identity := method(n,
    M := NMatrix clone dim(n,n)
    for(i,0,n-1, for(j,0,n-1, M atSet(i,j, if(i==j, 1, 0))))
    M
  )
  
  lenSquare := method(
    assert(columns==1)
    self dot(self)
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
    return P + (I-P) scaledBy(angle cos) + S scaledBy(angle sin)
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
  
  col := method( c,
    m := NMatrix clone dim(rows, 1)
    rows repeat(i, m atSet(i, self at(i, c)))
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
  M := NMatrix clone dim(call argCount, 1)
  values := call evalArgs
  
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
  
  mat := NMatrix clone dim(rows, cols)
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

NMatrix benchmark := method(
  ntests := 10 //2500
  "Benchmarking NMatrix" println
  a := list
  b := list
  ntests repeat(
    a append(vector(Random value, Random value, Random value))
    b append(vector(Random value, Random value, Random value))
  )
  t0 := Date now asNumber
  a foreach(i,v, v dot(b at(i)))
  t1 := Date now asNumber
  avg := (t1-t0)/ntests
  "  dot() performance in #{ntests} tests: #{(1/avg) asString(0,2)}/s (#{avg*1000}ms avg)" interpolate println
  
  a := list
  b := list
  ntests repeat(
    a append(Sequence clone setItemType("float64") set(Random value, Random value, Random value))
    b append(Sequence clone setItemType("float64") set(Random value, Random value, Random value))
  )
  t0 := Date now asNumber
  a foreach(i,v, v dotProduct(b at(i)))
  t1 := Date now asNumber
  avg := (t1-t0)/ntests
  "  Sequence dotProduct performance in #{ntests} tests: #{(1/avg) asString(0,2)}/s (#{avg*1000}ms avg)" interpolate println
  
  a := list
  ntests repeat(
    a append(matrix(Random value, Random value, Random value; Random value, Random value, Random value; Random value, Random value, Random value))
  )
  t0 := Date now asNumber
  a foreach(transpose)
  t1 := Date now asNumber
  avg := (t1-t0)/ntests
  "  transpose() performance in #{ntests} tests: #{(1/avg) asString(0,2)}/s (#{avg*1000}ms avg)" interpolate println

  a := list
  ntests repeat(
    a append(matrix(Random value, Random value, Random value; Random value, Random value, Random value; Random value, Random value, Random value))
  )
  t0 := Date now asNumber
  a foreach(transposeInPlace)
  t1 := Date now asNumber
  avg := (t1-t0)/ntests
  "  transposeInPlace() performance in #{ntests} tests: #{(1/avg) asString(0,2)}/s (#{avg*1000}ms avg)" interpolate println

  a := list
  b := list
  ntests repeat(
    a append(matrix(Random value, Random value, Random value; Random value, Random value, Random value; Random value, Random value, Random value))
    b append(matrix(Random value, Random value, Random value; Random value, Random value, Random value; Random value, Random value, Random value))
  )
  t0 := Date now asNumber
  a foreach(i,v, v matMult(b at(i)))
  t1 := Date now asNumber
  avg := (t1-t0)/ntests
  "  matMult() performance in #{ntests} tests: #{(1/avg) asString(0,2)}/s (#{avg*1000}ms avg)" interpolate println

)

NMatrix benchmark

