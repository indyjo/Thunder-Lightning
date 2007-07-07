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
