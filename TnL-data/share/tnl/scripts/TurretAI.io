AimHelper := Object clone do(
    with := method(s, t, w, tur,
        source = s
        target = t
        weapon = w
        turret = tur
        muzzle_speed_squared = weapon referenceSpeed squared
        requestTracer
        self
    )
    
    offset := vector(0,0,0)
    tracer := nil
    tracer_offset := nil
    weapon := nil
    turret := nil
    source := nil
    target := nil
    distance := Number constants inf
    muzzle_speed_squared := nil
    
    requestTracer := method(
        weapon onFireNotify(self)
    )
    weaponFired := method(
        tracer = weapon lastFiredRound
        tracer_offset = turret cannonDirWCS(source) - aimDir_initial(target location, target velocity)
    )
    
    aimDir_initial := method(target_p, target_v,
        origin := turret pivotWCS(source)
        delta_p := target_p - origin
        delta_p_norm := delta_p norm
        
        delta_v := target_v - source velocity
        delta_v_z_len := delta_p_norm dot(delta_v)
        delta_v_z := delta_p_norm scaledBy(delta_v_z_len)
        delta_v_xy := delta_v - delta_v_z
        bullet_speed := (muzzle_speed_squared - (delta_v_xy lenSquare)) max(0) sqrt
        #self eta := delta_p len / (bullet_speed - delta_v_z_len)
        
        result := delta_v_xy + delta_p_norm scaledBy(bullet_speed) # + vector(0,0.5*9.81*eta*eta,0)
        result norm
    )
    
    
    aimDir := method(
        dir := aimDir_initial(target location, target velocity) + offset
        dir norm
        #aimDir_initial(target location, target velocity)
    )
    
    update := method(
        origin := turret pivotWCS(source)
        to_target := target location - origin
        distance = to_target len

        tracer ifNil(
            return
        )
        tracer isAlive ifFalse(
            tracer = nil
            requestTracer
            return
        )
        
        to_tracer := tracer location - origin
        dir := to_target scaledBy(1/distance)
        if (to_tracer dot(dir) >= distance,
            dir_to_tracer := to_tracer norm
            dir_to_target := to_target norm
            
            delta := dir_to_tracer - dir_to_target
            #("Delta: " .. delta .. " length " .. (delta len)) say
            
            offset = offset + (tracer_offset - delta - offset) scaledBy(0.5)
            
            tracer = nil
            requestTracer
        )
    )
)

TurretAI := Object clone do(
  turret_control_name := "turret_steer"
  turret_state_name := "turret_angle"
  cannon_control_name := "cannon_steer"
  cannon_state_name := "cannon_angle"
  pivot := vector(0,0,0)
  turret_axis := vector(0,1,0)
  cannon_axis := vector(-1,0,0)
  reference := vector(0,0,1)
  weapon := nil
  
  worldToTurret := method(me, v,
    self hasSlot("LCS_inv") ifFalse(
        normal := turret_axis % reference
        self LCS_inv := matrix(  normal x,      normal y,      normal z
                                 turret_axis x, turret_axis y, turret_axis z
                                 reference x,   reference y,   reference z)
    )

    LCS_inv matMult( me orientation transpose matMult(v) )
  )
  
  pivotWCS := method(me,
    me location + me orientation matMult(pivot)
  )
  
  cannonDirLCS := method(me,
    turret := me controls float(turret_state_name)
    cannon := me controls float(cannon_state_name)
    
    vector(cannon cos * turret sin, cannon sin, cannon cos * turret cos)
  )
  
  cannonDirWCS := method(me,
    orient := me getOrientation
    y := orient matMult(turret_axis)
    z := orient matMult(reference)
    x := y % z
    
    dir := cannonDirLCS(me)
    x scaledBy(dir x) + y scaledBy(dir y) + z scaledBy(dir z)
  )
  
  CtlElement := coro(me, arg_target_angle, arg_control_name, arg_state_name, arg_factor,
    argDefaults(target_angle, arg_target_angle, 0)
    argDefaults(control_name, arg_control_name, "turret_steer", -1)
    argDefaults(state_name, arg_state_name, "turret_angle", -1)
    argDefaults(factor, arg_factor, 1)
    
    self error := Number constants inf
    integral := 0
    dt := nil
    
    loop(
      dt := pass
      
      error = target_angle - (me controls float(state_name))
      #(state_name .. " Target: " .. target_angle asString .. " state: ".. (me controls float(state_name) asString) .. " Error: " .. error) say
      while(error >  (Number pi), error = error - (Number pi * 2))
      while(error < (-Number pi), error = error + (Number pi * 2))
      
      integral = (integral + dt*error) clip(-0.5, 0.5)
      
      me controls setFloat(control_name, factor * (1.5 * error + 2*integral) clip(-1,1))
    )
  )

  AimRelative := coro(me, turret, arg_target_dir,
    if((self ?target_dir) isNil, self target_dir := arg_target_dir)
    
    self error := nil
    
    self cc := turret CtlElement clone start(me, nil, turret cannon_control_name, turret cannon_state_name)
    self ct := turret CtlElement clone start(me, nil, turret turret_control_name, turret turret_state_name)
    
    manage(cc)
    manage(ct)
    
    loop(
      pass
      error = (cc error squared + ct error squared) sqrt
      target_dir = target_dir norm
      #("Target_dir: " .. target_dir asString) say
      cc target_angle := target_dir y asin
      ct target_angle := target_dir x atan2(target_dir z)
    )
  )
  
  AimAbsolute := coro(me, turret, arg_target_dir,
    if((self ?target_dir) isNil, self target_dir := arg_target_dir)
    
    self error := nil
    
    self ar := turret AimRelative clone start(me, turret)
    manage(ar)
    
    loop(
      error = (ar ?error) ifNilEval(Number constants inf)
      ar target_dir := turret worldToTurret(me, self target_dir)
      pass
    )
  )
  
  AimAtAndFire := coro(me, turret, target,
    self aa := turret AimAbsolute clone start(me, turret, vector(1,0,0))
    manage(aa)
    
    weapon := turret weapon
    helper := AimHelper clone with(me, target, weapon, turret)
    
    ex := try(
      loop(
        helper update
        aa target_dir := helper aimDir
  
        # Let AimAbsolute do its work and compute its error
        pass
        
        if(aa error isNil not and aa error * helper distance < 8,
          weapon trigger
        ,
          weapon release
        )
      )
    )

    ex catch(InterruptedException,
      weapon release
      ex pass
    ) catch(
      ex pass
    )
  )
  
  AttackCloseTargets := coro(me, turret, arg_max_distance,
    argDefaults(max_distance, arg_max_distance, turret weapon maxRange)
    self target := nil
    had_target := false
    
    aaf := turret AimAtAndFire clone
    manage(aaf)

    loop(
      # See if our target is still close enough and alive. If not, set to nil
      if (target isNil not,
        dist := ((target getLocation) - (me getLocation)) len
        if(target isAlive not or dist > max_distance,
          target = nil
        )
      )
      
      # Start or interrupt the aimAndShoot coro as we need it
      if (target isNil not and had_target not and aaf running not,
        aaf start(me, turret, target)
        had_target = true
      )
      if (target isNil and had_target,
        aaf interrupt
        had_target = false
      )
      
      if (target isNil,
        candidates := Game queryActorsInSphere(me getLocation, max_distance)
        candidates selectInPlace(him, me dislikes(him))
        #"Candidates are #{candidates map(type)}" interpolate println
        candidates foreach(c,
          if (c type == "Drone",
            target = c
            break
          )
        )
      )

      sleep(1 + Random value)
    )
  )
) // AI

