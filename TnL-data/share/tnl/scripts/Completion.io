
Sequence do (
  updateSlot("beginsWithSeq", method(seq,
    self and (seq size <= size) and (exSlice(0,seq size) == seq)
  ))
  updateSlot("endsWithSeq", method(seq,
    self and (seq size <= size) and (exSlice(size - seq size) == seq)
  ))
)

Completion := Object clone do(
  openers := Map clone atPut(")","(") atPut("}","{") atPut("]","[")
  lastCommandOf := method(str,
    inString := nil
    inComment := nil
    braces := list
    stack := list(0)
    for(i, 0, str size - 1,
      c := str exSlice(i,i+1)
      if (inComment and c=="\n",
        inComment = nil; continue)
      if (inString and c=="\"" and str exSlice(i-1,i)  != "\\",
        inString = nil; continue)
      if (c == "#" or c=="/" and i != (str size - 1) and str exSlice(i+1,i+2) == "/",
        inComment = 1; continue)
      if (c == "(" or c=="{" or c=="[",
        stack push(i+1)
        braces push(c))
      if (c == ")" or c=="}" or c=="]",
        if (braces pop != openers at(c), return nil, stack pop))
      if (c == ";" or c=="\n" or c==",",
        stack pop
        stack push(i + 1))
    )
    if (inString or inComment, return nil)
    command := str exSlice(stack pop)
  )
    
  complete := method(str, context,
    command := lastCommandOf(str)
    if (command isNil, return list)
    
    try(
      msg := message(thisContext)
      if(command asMutable strip != "",
        msg setNext( command asMessage )
      )
    ) catch (
      return list
    )
    prefix := ""
    
    if (command endsWithSeq(" ") not and msg next, 
      m := msg
      while (m attachedMessage attachedMessage, m = m attachedMessage)
      prefix = m attachedMessage name
      m setAttachedMessage(nil)
    )
    
    res := softEval(context, msg)
    
    if (res,
      slots := allSlotNamesOf(res) select(i,v, v beginsWithSeq(prefix))
      slots mapInPlace (i,v, v exSlice(prefix size)) sort
    ,
      list)
  )
  
  // Tries to evaluate msg in context without doing anything that might
  // cause side effects.
  // If successful, returns the result
  // Else, returns nil
  softEval := block(context, msg,
    if (msg name == "" and msg arguments count == 0, return nil)
    obj := context
    while (msg,
      if (msg name asNumber) then (
        obj = msg name asNumber
      ) elseif (msg name beginsWithSeq("\"") and msg name endsWithSeq("\"")) then (
        obj = msg name unescape
      ) else (
        if (theslot := context getSlot( msg name ) ) then (
          if (msg name == "thisContext") then (
            obj = context
          ) elseif (theslot type == "CFunction") then (
            return nil
          ) elseif (theslot type == "Block") then (
            return nil
          ) else (
            obj = theslot
          )
        ) else (
          return nil
        )
      )
      msg = msg attachedMessage
    )
    return obj
  ) setIsActivatable(true)
  
  allSlotNamesOf := method(obj,
    slots := Map clone
    allSlotNamesOf_(obj, slots, List clone)
    found := slots keys
    // Treat Object and Lobby specially by allowing Object's slots to appear
    // in the list, which are normally suppressed
    if (obj == Object or obj == Lobby,
      found appendSeq( Object slotNames ))
    return found
  )
  
  allSlotNamesOf_ := method(obj, slots, visited,
    if (visited contains(obj), return)
    visited append(obj)

    // Don't visit past Object:
    if (obj == Object, return)
        
    obj protos foreach(i,v,
      allSlotNamesOf_(v, slots, visited))
    
    obj slotNames foreach(i,v,
      slots atPut(v,v))
    
    nil
  )
)
