Coro := Object clone do(
    InterruptedException := Exception clone
    
    init := method(
        self interruptReq := Nil
        self managed := list
    )
    
    start := method( @@wrapRun; self )
    
    doYield := method(
        yield
        if(interruptReq,
            InterruptedException raise(
                "interrupted",
                "The coroutine was interrupted")
        )
        self
    )
    
    interrupt := method(
        interruptReq = self
    )
    
    wrapRun := method(
        try(
            run
        ) catch (InterruptedException, ex,
            Nil
        )
        managed foreach(i,coro, coro interrupt)
    )
    
    manage := method(coro,
        managed add(coro)
        coro
    )
)
