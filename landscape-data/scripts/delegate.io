Object delegate := method(
  delegator := Object clone
  delegator subject := self
  delegator msgNames := List clone
  
  thisMessage arguments foreach(i,v, delegator msgNames add(v name))
    
  delegator to := method(
    context := thisMessage arguments at(0)
    context setAttachedMessage(
        message(performWithArgList( thisMessage name, thisMessage argsEvaluatedIn(sender) ) )
    )
    themethod := method(Nil) setMessage(context)
    msgNames foreach(i, name,
      subject setSlot(name, getSlot("themethod")))
  )
  
  delegator
)



