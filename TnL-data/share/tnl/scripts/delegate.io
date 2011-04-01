Object delegate := method(
  delegator := Object clone
  delegator subject := self
  delegator msgNames := List clone
  
  call message arguments foreach(i,v, delegator msgNames append(v name))
    
  delegator to := method(
    target := call message argAt(0) clone
    msg := message(call delegateTo) clone
    msg next appendArg(target)
    themethod := method() setMessage(msg)
    msgNames foreach(name,
      subject setSlot(name, getSlot("themethod"))
    )
  )
  
  delegator
)



