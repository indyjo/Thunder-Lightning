Object delegate := method(
  delegator := Object clone
  delegator subject := self
  delegator msgNames := List clone
  
  thisMessage arguments foreach(i,v, delegator msgNames append(v name))
    
  delegator to := method(
    context := thisMessage arguments at(0)
    
    msg := Message clone setName("")
    msg setArguments(list(context))
    msg setAttachedMessage(message( doMessage(thisMessage, sender) ) )
   
    themethod := method(Nil) setMessage(msg)
    msgNames foreach(i, name,
      subject setSlot(name, getSlot("themethod")))
  )
  
  delegator
)



