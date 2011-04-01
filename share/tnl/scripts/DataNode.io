# DataNode was already created by C++, so ad some convenience functions

DataNode do(
    getSlot := method(slotName,
        if(slotName at(2) == ":", switch(slotName at(0),
            "b", bool(slotName slice(2)),
            "f", float(slotName slice(2)),
            "i", int(slotName slice(2)),
            "s", string(slotName slice(2)),
            "v", vector(slotName slice(2)),
            nil)
        ,
            super(getSlot(slotName))
        )
    )
    
    setSlot := method(slotName, value,
        if(slotName at(2) == ":", switch(slotName at(0),
            "b", setBool(slotName slice(2), value),
            "f", setFloat(slotName slice(2), value),
            "i", setInt(slotName slice(2), value),
            "s", setString(slotName slice(2), value),
            "v", setVector(slotName slice(2), value),
            nil)
        ,
            super( setSlot(slotName, value) )
        )
        value
    )
)

