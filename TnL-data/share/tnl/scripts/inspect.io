Object do(
  identify := method(
    if (self isNil, return "nil")
    if (self == true, return "true")
    if (self == false, return "false")
    if (self == Object, return "Object")
    if (self == Lobby, return "Lobby")
    if (self == Protos, return "Protos")
    if (self == Core, return "Core")
    if (self == Addons, return "Addons")
    return "Object " .. uniqueHexId .. " ["..(self type).."]"
  )

  inspect := method(
    if(self hasSlot("proto") and self != Object, self proto inspect)
    methods := self slotNames select(i,v,
      (self getSlot(v) type == "CFunction") or (self getSlot(v) type == "Block")) sort
    data := self slotNames select(i,v,
      (self getSlot(v) type != "CFunction") and (self getSlot(v) type != "Block")) sort
    
    (self identify .. "\n") print
    write("Methods:\n")
    write(" ")
    cursor_pos := 1
    methods foreach(i,v,
      if( cursor_pos + (v size + 1) > 80,
        write("\n ")
	cursor_pos := 1
      )
      write(" " .. v)
      cursor_pos = cursor_pos + (v size + 1)
    )
    write("\n")

    write("Data:\n")
    write(" ")
    cursor_pos := 1
    data foreach(i,v,
      if( cursor_pos + (v size + 1) > 80,
        write("\n ")
        cursor_pos := 1
      )
      write(" " .. v)
      cursor_pos = cursor_pos + (v size + 1)
    )
    write("\n\n")
    
    nil
  )
)

