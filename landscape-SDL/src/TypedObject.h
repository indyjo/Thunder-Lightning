#ifndef TYPEDOBJECT_H
#define TYPEDOBJECT_H

class Type {
    const char * name;
    Type * parent;

public:
    Type(const char *name = "<unnamed>", Type & parent = *(Type*)0)
    :   name(name), parent(&parent)
    { }

    bool operator== (Type & other) { return &other == this; }
    const char * getName() { return name; }
    Type & getParent() { return *parent; }
};

class TypedObject {
    Type * type;
public:
    TypedObject(Type & type) : type(&type) { }
    Type & getType() { return *type; }

    bool instanceOf(Type & t) { return &t == type; }
    bool isA(Type & t) {
        Type * my_type = type;
        do {
            if (my_type == &t) return true;
            my_type = &my_type->getParent();
        } while (my_type);
        return false;
    }
};


#endif
