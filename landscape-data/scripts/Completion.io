String beginsWith := method(s, s == substring(0, s length))
String endsWith := method(s, s == substring(length - s length, length))

Completion := Object clone do(
  openers := Map clone atPut(")","(") atPut("}","{") atPut("]","[")
  lastCommandOf := method(str,
    inString := Nil
    inComment := Nil
    braces := list
    stack := list(0)
    for(i, 0, str length - 1,
      c := str substring(i,i+1)
      if (inComment and c=="\n",
        inComment = Nil; continue)
      if (inString and c=="\"" and str substring(i-1,i)  != "\\", 
        inString = Nil; continue)
      if (c == "#" or c=="/" and i != (str length - 1) and str substring(i+1,i+2) == "/",
        inComment = 1; continue)
      if (c == "(" or c=="{" or c=="[",
        stack push(i+1)
        braces push(c))
      if (c == ")" or c=="}" or c=="]",
        if (braces pop != openers at(c), return Nil, stack pop))
      if (c == ";" or c=="\n" or c==",",
        stack pop
        stack push(i + 1))
    )
    if (inString or inComment, return Nil)
    command := str substring(stack pop)
  )
    
  complete := method(str, context,
    command := lastCommandOf(str strip(" "))
    if (command isNil, return list)
    
    try(
      msg := message(thisContext) setAttachedMessage( command asMessage )
    ) catch (
      return list
    )
    prefix := ""
    
    if (command endsWith(" ") isNil, 
      m := msg
      while (m attachedMessage attachedMessage, m = m attachedMessage)
      prefix = m attachedMessage name
      m setAttachedMessage(Nil)
    )
    
    res := softEval(context, msg)
    if (res,
      slots := allSlotNamesOf(res) select(i,v, v beginsWith(prefix))
      slots map (i,v, v substring(prefix length)) sort
    ,
      list)
  )
  
  // Tries to evaluate msg in context without doing anything that might
  // cause side effects.
  // If successful, returns the result
  // Else, returns Nil
  softEval := block(context, msg,
    if (msg name == "" and msg arguments count == 0, return Nil)
    obj := context
    while (msg,
      if (msg name asNumber) then (
        obj = msg name asNumber
      ) elseif (msg name beginsWith("\"") and msg name endsWith("\"")) then (
        obj = msg name unescape
      ) else (
        if (theslot := context getSlot( msg name ) ) then (
          if (msg name == "thisContext") then (
            obj = context
          ) elseif (theslot type == "CFunction") then (
            return Nil
          ) elseif (theslot type == "Block") then (
            return Nil
          ) else (
            obj = theslot
          )
        ) else (
          return Nil
        )
      )
      msg = msg attachedMessage
    )
    return obj
  )
  
  allSlotNamesOf := block(obj,
    if (obj != Object and obj hasSlot("proto"),
      names := allSlotNamesOf(obj proto)
    ,
      names := list)
    obj slotNames foreach(i,v, if( names contains(v) isNil, names add(v)))
    return names
  )
)
