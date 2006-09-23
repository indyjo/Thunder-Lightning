assert := method(v,
  if (v,
    self
  ,
    code := thisMessage arguments at(0) code
    Exception raise("Assertion failed", "Assertion: " .. code)
  )
)

Matrix := Object clone do(
  rows := 1
  columns := 1
  entries := list(0)
  
  init := method(
    self rows := rows
    self columns := columns
    self entries := entries clone
  )

  at := method(r,c, entries at(c*rows+r))
  atSet := method(r,c,v, entries atPut(c*rows+r, v))

  print := method(
    write(rows,"x",columns,":\n");
    for(i,0,rows-1,
      write("|")
      for(j,0,columns-1,
        write(at(i,j))
        if (j!=columns-1, write(",\t"))
      )
      write("|\n")
    )
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
    args := thisMessage argsEvaluatedIn(sender)
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
    assert(other hasProto(Matrix))
    assert(columns == 1)
    assert(other rows == rows and other columns == 1)
    
    val := 0
    entries foreach(i,v, val = val + v * other entries at(i))
    val
  )
  
  setSlot("'", getSlot("transpose") )

  setSlot("+", method(other,
    assert(other hasProto(Matrix))
    assert(other rows == rows and other columns == columns)
    M := Matrix clone dim(rows,columns)
    M entries foreach(i,v,
      M entries atPut(i, self entries at(i) + other entries at (i)))
    M
  ))

  setSlot("-", method(other,
    assert(other hasProto(Matrix))
    assert(other rows == rows and other columns == columns)
    M := Matrix clone dim(rows,columns)
    M entries foreach(i,v,
      M entries atPut(i, self entries at(i) - other entries at (i)))
    M
  ))

  setSlot("*", method(other,
    if(other hasProto(Matrix),
      assert(self columns == other rows)
      M := Matrix clone dim(self rows, other columns)
      for(i,0,rows-1, for(j,0,other columns-1,
        x := 0
        for(k,0,columns-1, x=x+self at(i,k)*other at(k,j))
        M atSet(i,j,x)
      ))
      M
    ,
      M := Matrix clone dim(rows,columns)
      if(other hasProto(Number),
        self entries foreach(i,v, M entries atPut(i, v*other))
        M
      ,
        Exception raise("Type error","Invalid type for Matrix multiplication")
      )
    )
  ))
  
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
  
  norm := method(
    self * (1/self len)
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
)

vector := block(
  M := Matrix clone dim(thisMessage arguments size,1)
  values := thisMessage argsEvaluatedIn(sender)
  Lobby vals := values
  //writeln("values: ", values)
  M setList( values )
)

matrix := block(
  rows := List clone append(List clone)
  maxcols := 1
  thisMessage arguments foreach(i, arg,
    //writeln("argument ",i,": ",arg code)
    while(arg nextMessage,
      arg0 := arg clone
      arg0 setNextMessage(Nil)
      rows last append( sender doMessage(arg0) )
      if (rows last size > maxcols, maxcols = rows last size)
      rows append(List clone)
      arg = arg nextMessage
    )
    arg0 := arg clone
    arg0 setNextMessage(Nil)
    rows last append( sender doMessage(arg0) )
  )
  if (rows last size > maxcols, maxcols = rows last size)

  //writeln("Matrix dims: ",rows size," rows, ", maxcols, " columns.")
  M := Matrix clone dim(rows size, maxcols)
  rows foreach(i, row, row foreach(j, val,
    //writeln("entry at ",i,",",j,": ",val)
    M atSet(i,j, val)
  ))
  M
)

Number oldmult := Number getSlot("*")
Number setSlot("*", method(other,
  if(other hasProto(Matrix),
    M := Matrix clone dim(other rows, other columns)
    for(i,0,other rows - 1, for(j,0,other columns - 1,
      M atSet(i,j, oldmult(other at(i,j)))
    ))
    M
  ,
    oldmult(other)
  )
))
