Tank do (
  on("kill", ai interrupt; writeln("AI of Tank ", self identify, " interrupted"))
  //on("missileShot", self armament weapon("Cannon") trigger)

  location2 := method( vector(getLocation at(0,0), getLocation at(2,0)) )
  velocity2 := method( vector(getMovementVector at(0,0), getMovementVector at(2,0)) )
  front2 := method( f := getFrontVector; vector(f at(0,0), f at(2,0)) norm )
  right2 := method( f := front2; vector(f at(1,0), -f at(0,0)) )

  maintainSpeed := coro(me, arg_target_speed,
    self target_speed := arg_target_speed
    speed := me getMovementVector dot(me getFrontVector)
    diff := self target_speed - speed
    integral := 0
    while(1,
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
  
  maintainCourse := coro(me, arg_target_course,
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
    while(1,
      dt := pass
      old_error := error
      error := courseError
      
      derivative := ((error-old_error) / dt)
      me controls setFloat("steer", ((1.5 * error + 0.1*derivative) clip(-1,1)))
      if (me getMovementVector dot(me getFrontVector) < 0, me controls setFloat("steer", - me controls float("steer")))
      
      //writeln("course error: ", error, " to ", target_course entries," control:", me controls float("steer"))
    )
  )
  
  maintainVelocity := coro(me, arg_target_v,
    (self ?target_v) ifNil( self target_v := arg_target_v)
    
    self mc := Tank maintainCourse clone start(me, vector(0,0))
    manage(mc)
    self ms := Tank maintainSpeed clone start(me, 0)
    manage(ms)
    
    while(1,
      if( (target_v len) < 0.0001 ) then (
        mc target_course := me front2
      ) else ( mc target_course := self target_v norm )
      
      ms target_speed := target_v len
      
      //writeln("target v: ", target_v entries, " -> course ",mc target_course entries)
      
      pass
    )
  )
  
  maintainPosition := coro(me, arg_target_p, arg_target_v, arg_approach_speed,
    if ((self ?target_p) isNil, self target_p := target_p)
    if ((self ?target_v) isNil, self target_v := target_v)
    if ((self ?approach_speed) isNil,
      self approach_speed := if(arg_approach_speed isNil, 20.0, arg_approach_speed))
    
    mv := Tank maintainVelocity clone start(me)
    manage(mv)
    
    // The minimum distance at which an intercept course
    // is chosen. Below that distance, we will gradually
    // choose a parallel course.
    min_intercept_radius:=20
    tolerance_radius:=2
    
    while(1,
      p := me location2
      v := me velocity2
      
      dist2target := ((target_p - p) len)
      
      if (dist2target < tolerance_radius,
        mv target_v := target_v
        pass
        continue
      )
      
      
      dir2target := ((target_p - p) * (1/dist2target))
      // target velocity parallel
      tvpa := dir2target * dir2target dot(target_v)
      // target velocity perpendicular
      tvpe := target_v - tvpa
      
      intercept_v := tvpe + dir2target * ((approach_speed squared - tvpe lenSquare ) sqrt)
      t := (((dist2target - tolerance_radius) / (min_intercept_radius - tolerance_radius)) clip(0,1))
      
      mv target_v := ((1-t)*v + t*intercept_v)
      
      //writeln("target v: ", target_v entries, "target p:", target_p entries,
      //  " dp: ", (target_p-p) entries, " --> ", mv target_v entries)
      pass
    )
  )

  // A (hopefully better) variant of maintainPosition
  maintainPosition2 := coro(me, arg_target_p, arg_target_v,
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
    
    self mv := Tank maintainVelocity clone start(me)
    manage(mv)
    
    while(1,
      old_error = error
      p = me location2
      v = me velocity2
      
      error = target_p - p
      derivative = ((error-old_error) * (1/delta_t))
      
      mv target_v := target_v + Kp*error + Kd*derivative
      
      //writeln("target v: ", target_v entries, "target p:", target_p entries,
      //  " dp: ", (target_p-p) entries, " --> ", mv target_v entries)
      pass
    )
  )
  
  followPath := coro(me, arg_path, arg_closed, arg_speed,
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
    
    self mp := Tank maintainPosition2 clone start(me)
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
        a2 := a + t*(b-a)
        b2 := b + t*(c-b)
        t := positionOnSegment(a2,b2)
        a = a2
        b = b2
      )
      
      mp target_v := ((b - a) norm * speed)
      mp target_p := (a + ((b-a)*t))
      //writeln("mp target_p: ", mp target_p entries)
      //writeln("mp target_v: ", mp target_v entries)
      
      pass
    )
  )
  
  followTank := coro(me, arg_other, arg_x, arg_z,
    if((self ?other) isNil, self other := arg_other)
    if((self ?x) isNil, self x := arg_x)
    if((self ?z) isNil, self z := arg_z)
    
    self mp := Tank maintainPosition2 clone start(me)
    manage(mp)
    while(other isAlive,
      mp target_v := me velocity2
      mp target_p := other location2 + x*other right2 + z*other front2
      pass
    )
    
    mp interrupt
    me controls setFloat("throttle", 0)
    
  )
  
  flock := coro(me, arg_members, arg_avg_dist,
    (self ?members)  ifNil( self members := arg_members)
    (self ?avg_dist) ifNil( self avg_dist := arg_avg_dist)
    (self ?radius)   ifNil( self radius := 2*avg_dist)
    
    self mv := Tank maintainVelocity clone start(me)
    manage(mv)
    
    while(1,
      members selectInPlace(isAlive)
      neighbors := members select(n, (n != me) and ((n getLocation - me getLocation) lenSquare < radius*radius))
      
      avg_velocity := vector(0,0,0)
      gradient := vector(0,0,0)
      neighbors foreach(i,neighbor,
        d := neighbor getLocation - me getLocation
        dist := d len
        // gradient of the function (dist - avg_dist)^2
        gradient = gradient + 2*(dist - avg_dist)*(1/dist)*d
        avg_velocity := avg_velocity + neighbor getMovementVector
      )
      
      gradient2 := vector(gradient at(0,0), gradient at(2,0))
      
      if (neighbors size == 0,
        mv target_v := vector(0,0)
      ,
        avg_velocity = avg_velocity * (1/neighbors size)
        avg_velocity2 := vector(avg_velocity at(0,0), avg_velocity at (2,0))
        mv target_v := avg_velocity2 + gradient2
      )
      pass
    )
  )
  
  ctlTurret := coro(me, arg_target_angle,
    if((self ?target_angle) isNil, self target_angle := arg_target_angle)
    
    self error := nil
    integral := 0
    dt := nil
    
    loop(
      dt := pass
      
      error = target_angle - (me controls float("turret_angle"))
      while(error >  (Number pi), error = error - (Number pi * 2))
      while(error < (-Number pi), error = error + (Number pi * 2))
      
      integral = integral + dt*error
      
      me controls setFloat("turret_steer", (1.5 * error + 1*integral) clip(-1,1))
    )
  )

  ctlCannon := coro(me, arg_target_angle,
    if((self ?target_angle) isNil, self target_angle := arg_target_angle)
    
    self error := nil
    dt := nil
    
    integral := 0
    
    loop(
      dt = pass
      
      error = target_angle - (me controls float("cannon_angle"))
      while(error > (Number pi), error = error - (Number pi * 2))
      
      integral = integral + dt*error

      me controls setFloat("cannon_steer", (1.5 * error + 1*integral) clip(-1,1))
    )
  )

  aimRelative := coro(me, arg_target_dir,
    if((self ?target_dir) isNil, self target_dir := arg_target_dir)
    
    self error := nil
    
    self cc := Tank ctlCannon clone start(me)
    self ct := Tank ctlTurret clone start(me)
    
    manage(cc)
    manage(ct)
    
    loop(
      pass
      error = (cc error ifNilEval(0) squared + ct error ifNilEval(0) squared) sqrt
      target_dir = target_dir norm
      cc target_angle := target_dir at(1,0) atan
      ct target_angle := target_dir at(0,0) atan2(target_dir at(2,0))
    )
  )
  
  aimAbsolute := coro(me, arg_target_dir,
    if((self ?target_dir) isNil, self target_dir := arg_target_dir)
    
    self error := nil
    
    self ar := Tank aimRelative clone start(me)
    manage(ar)
    
    loop(
      error = ar ?error ifNilEval(Number constants inf)
      orient := me getOrientation transpose
      ar target_dir := orient * self target_dir
      pass
    )
  )
  
  aimAtAndFire := coro(me,
    
    self aa := Tank aimAbsolute clone start(me, vector(1,0,0))
    manage(aa)
    
    weapon := me armament weapon("Vulcan")
    #weapon setRoundsLeft(10000000)
    
    self target := nil
    delta_p := delta_p_norm := nil
    delta_v := delta_v_xy := delta_v_z := nil
    muzzle_speed_squared := Config Cannon_muzzle_velocity asNumber squared
    loop(
      if (target isNil not,
        dist := ((target getLocation) - (me getLocation)) len
        if(target isAlive and dist <= 2000,
          // still wrong: doesn't take into account the current turret/cannon state
          delta_p = ((target getLocation) - (me getLocation + 2.473674*(me getUpVector)))
          delta_p_norm = delta_p norm
          delta_v = target getMovementVector - me getMovementVector
          delta_v_z = delta_p_norm *(1 - (delta_v dot(delta_p_norm)))
          delta_v_xy = delta_v - delta_v_z
          bullet_speed := (muzzle_speed_squared - (delta_v_xy lenSquare)) sqrt
          self eta := delta_p len / (bullet_speed - delta_v dot(delta_p_norm))
          aa target_dir := delta_v_xy + delta_p_norm*bullet_speed + vector(0,0.5*9.81*eta*eta,0)

          if(aa error isNil not and aa error * delta_p len < 8 and delta_p len <= 1500,
            weapon trigger
          ,
            weapon release
          )
        ,
          target = nil
        )
      )
      if (target isNil,
        weapon release

        candidates := Game queryActorsInSphere(me getLocation, 2000)
        candidates selectInPlace(him, me dislikes(him))
        #"Candidates are #{candidates map(type)}" interpolate println
        candidates foreach(c,
          if (c type == "Drone",
            target = c
            break
          )
        )
      )
      pass
    )
  )
  
  
  ai := coro(me,
    #self aaf := Tank aimAtAndFire clone start(me)
    #manage(aaf)
    
    loop(
      pass
    )
  )
  init := method(
    self ai := ai clone start(self)
  )
)

Game postFrame := method(yield)

