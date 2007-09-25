TrajectoryRecorder := Object clone do(
    file := nil
    stop_requested := false
    seconds_per_sample := 1.0
    actor := nil
    
    with := method(actor_to_record, filename,
        actor = actor_to_record
        file = File clone with(filename)
        self
    )
    
    start := method( self @@runner; nil)
    stop := method(stop_requested = true)
    
    runner := method(
        file open
        seconds_left := 0
        
        while(stop_requested not,
            actor location entries appendSeq(actor orientation entries) foreach(v,
                file write(v asString .. " ")
            )
            file write("\n")
            
            seconds_left = seconds_left + seconds_per_sample
            while (seconds_left > 0,
                yield
                seconds_left = seconds_left - Clock getFrameDelta
            )
        )
        stop_requested = false
        
        file close
    )
)

