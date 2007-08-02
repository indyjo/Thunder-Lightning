CoroProfiles := Object clone do(
    profiles := list
    
    with := method(l,
        profiles = l clone
        
        self
    )
    
    asString := method(
        res := "CoroProfiles:\n"
        //             0         1         2         3         4         5         6
        //             0123456789012345678901234567890123456789012345678901234567890
        res := res .. "Name                            Sum     Passes T/pass    Instances\n"
        profiles foreach(p,
            d1 := p coro_type alignLeft(31)
            d2 := p time_sum asString(7,2)
            d3 := p passes asString(6,0)
            d4 := (1000 * p avg_time_per_pass) asString(9,2)
            d5 := p n_instances asString(5,0)
            res := res .. "#{d1} #{d2} #{d3} #{d4} #{d5}\n" interpolate
        )
        res
    )
    
    bySum := method( CoroProfiles clone with( profiles sortByKey(time_sum) ) )
    byName := method( CoroProfiles clone with( profiles sortByKey(coro_type) ) )
    byPasses := method( CoroProfiles clone with( profiles sortByKey(passes) ) )
    byInstances := method( CoroProfiles clone with( profiles sortByKey(n_instances) ) )
    byTimePerPass := method( CoroProfiles clone with( profiles sortByKey(avg_time_per_pass) ) )
)

Coro := Object clone do(
    InterruptedException := Exception clone
    
    init := method(
        self interruptReq := false
        self managed := list
        self running := nil
        self exception := nil
    )
    
    // Start the coroutine.
    start := method(
        @@wrapRun(call evalArgs)
        running = self
    )
    
    // Request that the coroutine be interrupted after its next call to pass()
    interrupt := method(
        interruptReq = true
    )
    
    // Demand that the coroutine interrupt another coroutine after it has finished.
    manage := method(coro,
        managed append(coro)
        coro
    )

    // Internal functions -- do not call from outside of run() !

    // Pass control to the next Io actor
    pass := method(
        sleep( 0.15 + 0.05 * Random value )
    )
    
    sleep := method(timetowait,
        dt := 0
        while(dt <= timetowait,
            yield
            if(interruptReq,
                interruptReq = false
                InterruptedException raise(
                    "interrupted",
                    "The coroutine was interrupted")
            )
            dt = dt+ Clock getFrameDelta
        )
        dt
    )
    
    wrapRun := method(arglist,
        ex := try(
            #writeln("run argumentNames: ",self getSlot("run") argumentNames)
            if (interruptReq not, performWithArgList("run", arglist))
        )
        ex catch (InterruptedException,
            nil
        ) catch (Exception,
            ex showStack
            self exception := ex
        ) catch (
            ex pass
        )
        running = nil
        managed foreach(i,coro, coro interrupt)
        managed empty
    )
    
    argDefaults := method(
        argName := call argAt(0) name
        defaultArgName := call argAt(1) name
        defaultValue := call evalArgAt(2)
      
        if (call sender getSlot(argName) isNil,
            call sender self setSlot(argName, call sender getSlot(defaultArgName) ifNilEval(defaultValue))
        )
    )
    
    enableProfiling := method(
        if (Coro hasSlot("profiles"), Exception raise("profiling already enabled"))
        
        
        Coro profiles := Map clone
        Coro oldPass := Coro getSlot("pass")
        
        Coro pass := method(
            cur_time := Date now asNumber
            key := self uniqueHexId
            profile := profiles at(key)
            if (profile isNil,
                profile := Object clone do(
                    time_sum := 0
                    time_of_last_pass := nil
                    passes := 0
                )
                profile coro_instance := self
                
                profiles atPut(key, profile)
            ,
                profile time_sum := profile time_sum + (cur_time - profile time_of_last_pass)
                profile passes := profile passes + 1
            )
            
            dt := oldPass
            profile time_of_last_pass := Date now asNumber
            dt
        )
        
        nil
    )
    
    getProfiles := method(
        coros := list()
        profiles foreach(p,
            c := Object clone
            c coro_type := p coro_instance uniqueType
            c time_sum := p time_sum
            c passes := p passes
            coros append(c)
        )
        
        coros = coros sortByKey(coro_type)
        
        coros_by_type := list()
        current_type := nil
        coros foreach(c,
            if (c coro_type != current_type,
                coros_by_type append(Object clone)
                coros_by_type last coro_type := c coro_type
                coros_by_type last n_instances := 0
                coros_by_type last time_sum := 0
                coros_by_type last passes := 0
                current_type := c coro_type
            )
            coros_by_type last n_instances := coros_by_type last n_instances + 1
            coros_by_type last time_sum := coros_by_type last time_sum + c time_sum
            coros_by_type last passes := coros_by_type last passes + c passes
            coros_by_type last avg_time_per_pass := coros_by_type last time_sum / coros_by_type last passes
        )
        
        coros_by_type = coros_by_type sortByKey(time_sum)
        CoroProfiles clone with(coros_by_type)
    )
    
    uniqueType := method(
        if(self hasSlot("tag"),
            self type .. " (" .. self tag ..")"
        ,
            self type
        )
    )
)


Object coro := method(
    //("message: " .. call message) println
    //Coroutine currentCoroutine showStack
    msg := call message arguments last
    
    argNames := call message arguments slice(0, call argCount - 1 )
    argNames mapInPlace(m, m name)
    
    c := Coro clone
    c run := method()
    c getSlot("run") setArgumentNames(argNames)
    //writeln("coro argumentNames: ",c getSlot("run") argumentNames)
    //("msg: " .. msg) println
    c getSlot("run") setMessage(msg)
    c
)
