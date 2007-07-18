#ifndef TYPEDOBJECT_H
#define TYPEDOBJECT_H

class Type {
public:
	typedef const Type *const List;
private:
	static const List noparents;
	static const char *noname;
    const char * name;
    /// Pointer to a zero-terminated list of parents.
    const List * parents;

public:
    Type(const char *name, ...);

    inline bool operator== (const Type & other) const { return &other == this; }
    inline const char * getName() const { return name; }
    inline List * getParents() const { return parents; }
    
    bool isA(const Type & t) const;
};

class TypedObject {
    const Type * type;
public:
    TypedObject(const Type & type) : type(&type) { }
    
    inline const Type & getType() const { return *type; }
    inline void setType(const Type & t) { type = &t; }

    inline bool instanceOf(const Type & t) { return &t == type; }
    inline bool isA(const Type & t) { return type->isA(t); }
};


#endif
