Object delegate := method(
  delegator := Object clone
  delegator subject := self
  delegator msgNames := List clone
  
  call message arguments foreach(i,v, delegator msgNames append(v name))
    
  delegator to := method(
    context := call argAt(0)
    
    msg := Message clone setName("")
    msg setArguments(list(context))
    msg setAttachedMessage(message( doMessage(call message, call sender) ) )
   
    themethod := method(Nil) setMessage(msg)
    msgNames foreach(i, name,
      subject setSlot(name, getSlot("themethod")))
  )
  
  delegator
)



