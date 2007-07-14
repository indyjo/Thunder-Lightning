Matrix

NavPath := Object clone do(
    path := nil
    normals := nil
    
    with := method(points,
        self path = points clone
        self normals = calculateNormals
        self
    )
    
    size := method(
        path size
    )
    
    done := method(
        size == 1
    )
    
    // For every path point, calculate a the normal vector of a plane dividing
    // the space between two path segments with equal angles.
    // The plane normals will always point towards the next path point.
    // Works for 2D and 3D paths
    calculateNormals := method(
        normals := list
        normals append((path at(1) - path at(0)) norm)
        (path size - 2) repeat(i,
            prev := path at(i)
            p := path at(i+1)
            next := path at(i+2)
            
            d1 := (p - prev) norm
            d2 := (next - p) norm
            
            normals append( (d1+d2) norm )
        )
        
        normals append((path at(path size -1) - path at(path size -2)) norm)
    )
    
    // Projects a vector (x) onto the first segment of a path, resulting in a
    // float value u. If 0<=u<=1, then x is "near" the first path segment.
    // if u < 0, then it lies "before" the first segment. If u > 1, then it is
    // "beyond" the first segment.
    projectToFirstSegment := method(x,
        a := (x-path at(0)) dot( normals at(0) )
        b := (x-path at(1)) dot( normals at(1) )
        if ( a == -b, return 1.5 )
        a / (a-b)
    )
    
    // Projects a point onto the path.
    // This will modify the path, removing points from the front until the
    // current segment is found.
    projectToPath := method(x,
        u := projectToFirstSegment(x)
        while( u > 1,
            path removeFirst
            normals removeFirst
            if (path size == 1, break)
            u := projectToFirstSegment(x)
        )
        u
    )
    
    // Returns a line segment that serves as reference for following the path.
    // The line segment will be smoothly interpolated between navpath points.
    // This will modify the path, removing points from the front until the
    // current segment is found.
    currentSegmentSmoothed := method(x,
        if (path size > 1, u := projectToPath(x))
        
        if (path size > 2) then (
            a := path at(0) mixedWith(path at(1), u)
            b := path at(1) mixedWith(path at(2), u)
        ) elseif(path size == 2) then(
            a := path at(0)
            b := path at(1)
        ) else (
            a := path at(0)
            b := a + normals at(0)
        )
        list(a,b)
    )
)

