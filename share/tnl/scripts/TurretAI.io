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
    orient := me getOrientation
    wcs_turret_axis := orient matMult(turret_axis)
    wcs_reference := orient matMult(reference)
    wcs_normal := wcs_turret_axis % wcs_reference

    # wcs_normal, wcs_turret_axis and wcs_reference define a coordinate system.
    # We form the inverse matrix, consisting of these vectors as rows.
    m := matrix(  wcs_normal at(0),       wcs_normal at(1),       wcs_normal at(2)
                  wcs_turret_axis at(0),  wcs_turret_axis at(1),  wcs_turret_axis at(2)
                  wcs_reference at(0),    wcs_reference at(1),    wcs_reference at(2))

    m matMult(v)
  )
  
  CtlElement := coro(me, arg_target_angle, arg_control_name, arg_state_name, arg_factor,
    argDefaults(target_angle, arg_target_angle, 0)
    argDefaults(control_name, arg_control_name, "turret_steer", -1)
    argDefaults(state_name, arg_state_name, "turret_angle", -1)
    argDefaults(factor, arg_factor, 1)
    
    self error := nil
    integral := 0
    dt := nil
    
    loop(
      dt := pass
      
      error = target_angle - (me controls float(state_name))
      #(state_name .. " Target: " .. target_angle asString .. " state: ".. (me controls float(state_name) asString) .. " Error: " .. error) say
      while(error >  (Number pi), error = error - (Number pi * 2))
      while(error < (-Number pi), error = error + (Number pi * 2))
      
      integral = integral + dt*error
      
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
      error = (cc error ifNilEval(0) squared + ct error ifNilEval(0) squared) sqrt
      target_dir = target_dir norm
      #("Target_dir: " .. target_dir asString) say
      cc target_angle := target_dir at(1,0) asin
      ct target_angle := target_dir at(0,0) atan2(target_dir at(2,0))
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
    
    delta_p := delta_p_norm := nil
    delta_v := delta_v_xy := delta_v_z := nil
    
    // TODO: we should be able to read this from the weapon
    muzzle_speed_squared := Config Cannon_muzzle_velocity asNumber squared
    
    ex := try(
      loop(
        delta_p = ((target getLocation) - (me getLocation + me getOrientation matMult(turret pivot) ))
        delta_p_norm = delta_p norm
        delta_v = target getMovementVector - me getMovementVector
        delta_v_z = delta_p_norm scaledBy(1 - (delta_v dot(delta_p_norm)))
        delta_v_xy = delta_v - delta_v_z
        bullet_speed := (muzzle_speed_squared - (delta_v_xy lenSquare)) sqrt
        self eta := delta_p len / (bullet_speed - delta_v dot(delta_p_norm))
        aa target_dir := delta_v_xy + delta_p_norm scaledBy(bullet_speed) + vector(0,0.5*9.81*eta*eta,0)
  
        # Let AimAbsolute do its work and compute its error
        pass
        
        if(aa error isNil not and aa error * delta_p len < 8,
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

