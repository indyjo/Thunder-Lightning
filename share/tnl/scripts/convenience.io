Config do(
    old_setSlot := getSlot("setSlot")

    // This definition makes Config call query(key) for every slot not found in Config
    forward := method(
        query(call message name)
    )

    // The following line change the behavior of "setSlot" and "setSlotWithType"
    // to call the set(key, value) method.
    // This is very tricky, don't modify if you don't have to!
    configSetSlot := method(key, value,
        "In setSlot!" println
        if (hasSlot(key), old_setSlot(key,value), set(key,value))
        write("Config: " .. key .. "=" .. value .. "\n")
        value
    )
    setSlotWithType := getSlot("configSetSlot")
    setSlot := getSlot("configSetSlot")
)
