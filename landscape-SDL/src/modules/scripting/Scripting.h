#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <string>
#include <vector>

#include <object.h>

#define DECLARE_TRAITS(T)                               \
    namespace Scripting {                               \
    template<> struct Traits<T> {                       \
        static const Class * type;                      \
        static const Class * query_type;                \
    };                                                  \
    }
#define DEFINE_TRAITS(T, typeobj)                       \
    namespace Scripting {                               \
        const Class * Traits<T>::type = & typeobj;      \
        const Class * Traits<T>::query_type = & typeobj;\
    }

namespace Scripting {

    struct Class;
    struct Func;
    struct Tuple;
    struct Any;

    struct UndefinedTraits {
        static const Class * query_type;
    };

    template<class T> struct Traits : public UndefinedTraits
    {
    };
}

DECLARE_TRAITS(void)
DECLARE_TRAITS(Any)
DECLARE_TRAITS(int)
DECLARE_TRAITS(float)
DECLARE_TRAITS(std::string)
DECLARE_TRAITS(Ptr<Tuple>)

namespace Scripting {
    struct Exception : public std::exception {
        std::string str;
        Exception (const char * str) throw() : str(str) { }
        ~Exception() throw() { }
        const char * what() const throw() { return str.c_str(); }
    };

    struct ValueBase : public Object {
        const Class * type;
        ValueBase(const Class * type) : type(type) { };
        inline const Class * getType() { return type; }
        inline bool instanceOf(const Class * t) { return type == t; }
        template<class U> inline bool is()
        { return type == Traits<U>::query_type; }

        virtual ~ValueBase() { };
    };

    template<class T> bool instanceOf(Ptr<ValueBase> v) {
        return v->getType() == Traits<T>::query_type;
    }

    template<class T>
    struct Value : public ValueBase {
        Value(T t) : ValueBase(Traits<T>::type), val(t) { }

        T & get() { return val; }
        void set(T t) { val = t; }
    protected:
        T val;
    };

    template<class T>
    inline Ptr<ValueBase> make_val(T t) { return new Value<T>(t); }

    template<>
    inline Ptr<ValueBase> make_val(const char *c)
    { return make_val(std::string(c)); }

    template<class T>
    inline Ptr<ValueBase> make_val_ptr(T * t)
    { return make_val(Ptr<T>(t)); }
    
    inline Ptr<ValueBase> make_void() {
        return new ValueBase(Traits<void>::type);
    }

    template<class T>
    const T & get(Ptr<ValueBase> v, T default_val = T()) {
        if(instanceOf<T>(v)) {
            Value<T> & vt = reinterpret_cast<Value<T> &> (*v);
            return vt.get();
        } else {
            return default_val;
        }
    }

    template<class T>
    const Ptr<T> & getPtr(Ptr<ValueBase> v) { return get<Ptr<T> >(v); }

} // namespace Scripting

#endif
