Tank coro := method(
    res := resend
    res tag := "Tank"
    res
)

Tank do (
  init := method(
    self turret := TurretAI clone do(
      pivot := vector(0, 2.336049, -1.867034)
    )
    turret weapon := armament weapon("Vulcan")
  )
  
  isGroundTarget := true
  

  MaintainSpeed := coro(me, arg_target_speed,
    self target_speed := arg_target_speed
    speed := me getMovementVector dot(me getFrontVector)
    diff := self target_speed - speed
    integral := 0
    loop(
      dt := pass
      old_diff := diff
      speed := me getMovementVector dot(me getFrontVector)
      diff := self target_speed - speed
      
      integral   := ((integral + dt*diff) clip (-0.3,0.3))
      derivative := ((diff-old_diff) / dt)
      
      me controls setFloat("throttle", ((1.5 * diff + 2.5 * integral + 0.005 * derivative) clip(-1,1)))
      //writeln("speed: ",speed," target: ",self target_speed," control: ", me controls float("throttle"),
      //        " int: ",integral," deriv: ", derivative)
    )
  )
  
  MaintainCourse := coro(me, arg_target_course,
    if ((self ?target_course) isNil, self target_course := arg_target_course)
    courseError := block(
      front := me front2
      right := me right2
      error := right dot(self target_course)
      if (front dot(self target_course) < 0,
        error := if(error >= 0, 1, -1))
      error
    ) setIsActivatable(true)
 
    error := courseError
    loop(
      dt := pass
      old_error := error
      error := courseError
      
      derivative := ((error-old_error) / dt)
      me controls setFloat("steer", ((1.5 * error + 0.1*derivative) clip(-1,1)))
      if (me getMovementVector dot(me getFrontVector) < 0, me controls setFloat("steer", - me controls float("steer")))
      
      //writeln("course error: ", error, " to ", target_course entries," control:", me controls float("steer"))
    )
  )
  
  MaintainVelocity := coro(me, arg_target_v,
    (self ?target_v) ifNil( self target_v := arg_target_v)
    
    self mc := Tank MaintainCourse clone start(me, vector(0,0))
    manage(mc)
    self ms := Tank MaintainSpeed clone start(me, 0)
    manage(ms)
    
    loop(
      if( (target_v len) < 0.0001 ) then (
        mc target_course := me front2
      ) else ( mc target_course := self target_v norm )
      
      ms target_speed := target_v len
      
      //writeln("target v: ", target_v entries, " -> course ",mc target_course entries)
      
      pass
    )
  )
  
  MaintainPosition := coro(me, arg_target_p, arg_target_v, arg_approach_speed,
    if ((self ?target_p) isNil, self target_p := target_p)
    if ((self ?target_v) isNil, self target_v := target_v)
    if ((self ?approach_speed) isNil,
      self approach_speed := if(arg_approach_speed isNil, 20.0, arg_approach_speed))
    
    mv := Tank MaintainVelocity clone start(me)
    manage(mv)
    
    // The minimum distance at which an intercept course
    // is chosen. Below that distance, we will gradually
    // choose a parallel course.
    min_intercept_radius:=20
    tolerance_radius:=2
    
    loop(
      p := me location2
      v := me velocity2
      
      dist2target := (target_p - p) len
      
      if (dist2target < tolerance_radius,
        mv target_v := target_v
        pass
        continue
      )
      
      
      dir2target := (target_p - p) scaledBy(1/dist2target)
      // target velocity parallel
      tvpa := target_v projectedOn(dir2target)
      // target velocity perpendicular
      tvpe := target_v - tvpa
      
      intercept_v := tvpe + dir2target scaledBy((approach_speed squared - tvpe lenSquare ) sqrt)
      t := (((dist2target - tolerance_radius) / (min_intercept_radius - tolerance_radius)) clip(0,1))
      
      mv target_v := v mixedWith(intercept_v, t)
      
      //writeln("target v: ", target_v entries, "target p:", target_p entries,
      //  " dp: ", (target_p-p) entries, " --> ", mv target_v entries)
      pass
    )
  )

  // A (hopefully better) variant of MaintainPosition
  MaintainPosition2 := coro(me, arg_target_p, arg_target_v,
    if ((self ?target_p) isNil, self target_p := target_p)
    if ((self ?target_v) isNil, self target_v := target_v)
    (self ?Kp) ifNil(self Kp := 1)
    (self ?Kd) ifNil(self Kd := 0.1)
    
    p := me location2
    v := me velocity2
    error := target_p - p
    derivative := nil
    old_error := nil
    
    delta_t := pass
    
    self mv := Tank MaintainVelocity clone start(me)
    manage(mv)
    
    loop(
      old_error = error
      p = me location2
      v = me velocity2
      
      error = target_p - p
      derivative = (error-old_error) scaledBy(1/delta_t)
      
      mv target_v := target_v + error scaledBy(Kp) + derivative scaledBy(Kd)
      
      //writeln("target v: ", target_v entries, "target p:", target_p entries,
      //  " dp: ", (target_p-p) entries, " --> ", mv target_v entries)
      pass
    )
  )
  
  FollowPath := coro(me, arg_path, arg_closed, arg_speed,
    if((self ?vpath) isNil, self vpath := arg_path)
    if((self ?closed) isNil, self closed := if (arg_closed isNil, self, arg_closed))
    if((self ?speed) isNil, self speed := if (arg_speed isNil, 15.0, arg_speed))
    
    positionOnSegment := block(a,b,
      d := b - a
      p := me location2
      (p - a) dot(d) / d lenSquare
    ) setIsActivatable(true)
    
    popSegment := block(
      front := vpath removeAt(0)
      if (closed, vpath push(front))
    ) setIsActivatable(true)
    
    self mp := Tank MaintainPosition2 clone start(me)
    manage(mp)
    
    while (path size > 1,
      a := vpath at(0)
      b := vpath at(1)
      t := positionOnSegment(a,b)
      //writeln("path segment: ",a entries," -> ", b entries, " t: ",t)
      if ( t > 1,
        popSegment
        continue
      )
      if ((a-b) lenSquare < 0.0001,
        popSegment
        continue
      )
      if (vpath size > 2,
        if( positionOnSegment(b,vpath at(2)) between(0,1),
          popSegment
          continue
        )
        c := vpath at(2)
        a2 := a mixedWith(b, t)
        b2 := b mixedWith(c, t)
        t := positionOnSegment(a2,b2)
        a = a2
        b = b2
      )
      
      mp target_v := (b - a) norm scaledBy(speed)
      mp target_p := a mixedWith(b, t)
      //writeln("mp target_p: ", mp target_p entries)
      //writeln("mp target_v: ", mp target_v entries)
      
      pass
    )
  )
  
  FollowTank := coro(me, arg_other, arg_x, arg_z,
    if((self ?other) isNil, self other := arg_other)
    if((self ?x) isNil, self x := arg_x)
    if((self ?z) isNil, self z := arg_z)
    
    self mp := Tank MaintainPosition2 clone start(me)
    manage(mp)
    while(other isAlive,
      mp target_v := me velocity2
      mp target_p := other location2 + other right2 scaledBy(x) + other front2 scaledBy(z)
      pass
    )
    
    mp interrupt
    me controls setFloat("throttle", 0)
    
  )
  
  Flock := coro(me, arg_members, arg_avg_dist,
    (self ?members)  ifNil( self members := arg_members)
    (self ?avg_dist) ifNil( self avg_dist := arg_avg_dist)
    (self ?radius)   ifNil( self radius := 2*avg_dist)
    
    self mv := Tank MaintainVelocity clone start(me)
    manage(mv)
    
    loop(
      members selectInPlace(isAlive)
      neighbors := members select(n, (n != me) and ((n getLocation - me getLocation) lenSquare < radius*radius))
      
      avg_velocity := vector(0,0,0)
      gradient := vector(0,0,0)
      neighbors foreach(i,neighbor,
        d := neighbor getLocation - me getLocation
        dist := d len
        d scaleInPlaceBy(1/dist)
        // gradient of the function (dist - avg_dist)^2
        gradient = gradient + d scaledBy( 2*(dist - avg_dist) )
        avg_velocity := avg_velocity + neighbor getMovementVector
      )
      
      gradient2 := vector(gradient at(0,0), gradient at(2,0))
      
      if (neighbors size == 0,
        mv target_v := vector(0,0)
      ,
        avg_velocity = avg_velocity scaledBy(1/neighbors size)
        avg_velocity2 := vector(avg_velocity at(0,0), avg_velocity at (2,0))
        mv target_v := avg_velocity2 + gradient2
      )
      pass
    )
  )
  
  ai := coro(me,
    self act := me turret AttackCloseTargets clone start(me, me turret)
    manage(act)
    
    loop(
      pass
    )
  )
  
  on("start_ai",
    self _ai := ai clone start(self)
    ("AI of Tank ".. self uniqueHexId .. " started") println
  )
  on("stop_ai",
    _ai interrupt
    ("AI of Tank ".. self uniqueHexId .. " interrupted") println
  )

)

Game postFrame := method(yield)

