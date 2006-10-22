Coro := Object clone do(
    InterruptedException := Exception clone
    
    init := method(
        self interruptReq := Nil
        self managed := list
        self running := Nil
    )
    
    // Start the coroutine.
    start := method(
        @@wrapRun(call evalArgs)
        running = self
    )
    
    // Request that the coroutine be interrupted after its next call to pass()
    interrupt := method(
        interruptReq = self
    )
    
    // Demand that the coroutine interrupt another coroutine after it has finished.
    manage := method(coro,
        managed append(coro)
        coro
    )

    // Internal functions -- do not call from outside of run() !

    // Pass control to the next Io actor
    pass := method(
        sleep( 0.25 + 0.05 * Random value )
    )
    
    sleep := method(timetowait,
        dt := 0
        while(dt <= timetowait,
            yield
            if(interruptReq,
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
            performWithArgList("run", arglist)
        )
        ex catch (InterruptedException,
            Nil
        ) catch (Exception,
            ex showStack
        )
        running = Nil
        managed foreach(i,coro, coro interrupt)
    )
    
)

Object coro := method(
    msg := call message arguments last
    
    argNames := call message arguments slice(0, call argCount - 2 )
    argNames mapInPlace(m, m name)

    c := Coro clone
    c run := method(Nop)
    c getSlot("run") setArgumentNames(argNames)
    c getSlot("run") setMessage(msg)
    
    c
}
