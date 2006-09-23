Coro := Object clone do(
    InterruptedException := Exception clone
    
    init := method(
        self interruptReq := Nil
        self managed := list
        self running := Nil
    )
    
    // Start the coroutine.
    start := method(
        @@wrapRun(thisMessage argsEvaluatedIn(sender))
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
        try(
            performWithArgList("run", arglist)
        ) catch (InterruptedException, ex,
            Nil
        ) catch (Exception, ex,
            writeln(ex name, ": ", ex description)
            writeln(ex backTraceString)
        )
        running = Nil
        managed foreach(i,coro, coro interrupt)
    )
    
)

Object coro := method(
    msg := thisMessage arguments last
    
    argNames := thisMessage arguments slice(0, thisMessage arguments size - 2 )
    argNames mapInPlace(m, m name)

    c := Coro clone
    c run := method(Nop)
    c getSlot("run") setArgumentNames(argNames)
    c getSlot("run") setMessage(msg)
    
    c
}
