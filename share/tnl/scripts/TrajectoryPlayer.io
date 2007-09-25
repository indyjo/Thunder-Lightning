"Again" println
TrajectoryPlayer := Object clone do(
    file := nil
    is_running := false
    stop_requested := false
    actor := nil
    is_looping := false
    
    with := method( actor_to_control, filename,
        actor = actor_to_control
        file = File clone with(filename) openForReading
        self
    )
    
    isRunning := method(is_running)
    start := method(
        is_running = true
        @@runner
        self
    )
    stop := method(
        stop_requested = true
        self
    )
    
    runner := method(
        values := list()
        while((line := file readLine) isNil not,
            values append(line split map(asNumber))
        )
        
        t := 0
        current := list(0,0,0,0,0,0,0,0,0,0,0,0)

        while( values size >= 4 and stop_requested not,
            12 repeat(i,
                current atPut(i, catmull_rom(t,
                    values at(0) at(i),
                    values at(1) at(i),
                    values at(2) at(i),
                    values at(3) at(i)))
            )
            
            // We don't copy the matrix values directly because we need more
            // control over normalization. Otherwise, we would often get some
            // ugly artefacts.
            front := Matrix clone dim(3,1) setList(current slice(9,12)) norm
            up := Matrix clone dim(3,1) setList(current slice(6,9)) norm
            right := (up % front) norm
            up := (front % right) norm
            entries := right entries clone appendSeq(up entries) appendSeq(front entries)
            
            location := Matrix clone dim(3,1) setList(current slice(0,3))
            orient   := Matrix clone dim(3,3) setList(entries)
            actor setLocation(location)
            actor setOrientation(orient)
            
            yield
            
            t = t + Clock getFrameDelta
            while(t > 1 and values isEmpty not,
                if (is_looping,
                    values append(values removeFirst)
                ,
                    values removeFirst
                )
                t = t - 1
            )
        )
        
        is_running = false
        stop_requested = false
    )
    
    weight_matrix := matrix( 0, 2, 0, 0
                            -1, 0, 1, 0
                             2,-5, 4,-1
                            -1, 3,-3, 1)
    catmull_rom := method(t, p0, p1, p2, p3,
        vector(1,t,t*t,t*t*t) dot(weight_matrix matMult(vector(p0,p1,p2,p3))) * 0.5
    )
)

