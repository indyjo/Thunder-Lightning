#ifndef SCRIPTING_FUNCIMPL_H
#define SCRIPTING_FUNCIMPL_H

#include <map>
#include <string>
#include "Func.h"

namespace Scripting {
    template<class R, class T>
    struct Func0 : public FuncImpl {
        typedef R (T::* FuncType) ();
        FuncType func;

        Func0(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            return make_val(((*obj).*func)());
        }
    };
    template<class T>
    struct Func0<void, T> : public FuncImpl {
        typedef void (T::* FuncType) ();
        FuncType func;

        Func0(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            ((*obj).*func)();
            return make_void();
        }
    };
    
    template<class R, class T, class P1>
    struct Func1 : public FuncImpl {
        typedef R (T::* FuncType) (P1);
        FuncType func;
        
        Func1(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            P1 p1 = get<P1>(args[0]);
            return make_val(((&*obj)->*func)(p1));
        }
    };
    template<class T, class P1>
    struct Func1<void, T, P1> : public FuncImpl {
        typedef void (T::* FuncType) (P1);
        FuncType func;

        Func1(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            P1 p1 = get<P1>(args[0]);
            ((*obj).*func)(p1);
            return make_void();
        }
    };
    
    template<class R, class T, class P1, class P2>
    struct Func2 : public FuncImpl {
        typedef R (T::* FuncType) (P1, P2);
        FuncType func;

        Func2(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            P1 p1 = get<P1>(args[0]);
            P2 p2 = get<P2>(args[1]);
            return make_val(((&*obj)->*func)(p1,p2));
        }
    };
    template<class T, class P1, class P2>
    struct Func2<void, T, P1, P2> : public FuncImpl {
        typedef void (T::* FuncType) (P1, P2);
        FuncType func;

        Func2(FuncType func) : func(func) { };

        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            P1 p1 = get<P1>(args[0]);
            P2 p2 = get<P2>(args[0]);
            ((*obj).*func)(p1, p2);
            return make_void();
        }
    };

    
    template<class R>
    struct StaticFunc0 : public FuncImpl {
        typedef R (*FuncType) ();
        FuncType func;
        
        StaticFunc0(FuncType func) : func(func) { };
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            return make_val(func());
        }
    };

    template<class R, class P1>
    struct StaticFunc1 : public FuncImpl {
        typedef R (*FuncType) (P1);
        FuncType func;
        
        StaticFunc1(FuncType func) : func(func) { };
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            P1 p1 = get<P1>(args[0]);
            return make_val(func(p1));
        }
    };

    template<class R, class P1, class P2>
    struct StaticFunc2 : public FuncImpl {
        typedef R (*FuncType) (P1, P2);
        FuncType func;
        
        StaticFunc2(FuncType func) : func(func) { };
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            P1 p1 = get<P1>(args[0]);
            P2 p2 = get<P2>(args[1]);
            return make_val(func(p1,p2));
        }
    };

    template<class R, class T>
    Ptr<FuncImpl> make_impl(R (T::* func) ()) {
        return new Func0<R,T>(func);
    }
    
    template<class R, class T, class P1>
    Ptr<FuncImpl> make_impl(R (T::* func) (P1)) {
        return new Func1<R,T,P1>(func);
    }

    template<class R, class T, class P1, class P2>
    Ptr<FuncImpl> make_impl(R (T::* func) (P1, P2)) {
        return new Func2<R,T,P1,P2>(func);
    }
    
    template<class R>
    Ptr<FuncImpl> make_impl(R (*func) ()) {
        return new StaticFunc0<R>(func);
    }

    template<class R, class P1>
    Ptr<FuncImpl> make_impl(R (*func) (P1 p1)) {
        return new StaticFunc1<R,P1>(func);
    }

    template<class R, class P1, class P2>
    Ptr<FuncImpl> make_impl(R (*func) (P1 p1, P2 p2)) {
        return new StaticFunc2<R,P1,P2>(func);
    }
    
    template<class R, class T>
    struct Getter : public FuncImpl {
        typedef R T::* MemberType;
        MemberType member;
        
        Getter(MemberType member) : member(member) { }
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Ptr<T> obj = get<Ptr<T> >(context);
            return make_val((&*obj)->*member);
        }
    };
    
    template<class R, class T>
    Ptr<FuncImpl> make_getter(R T::* member) {
        return new Getter<R,T>(member);
    }
    
    template<class Src, class Dst>
    struct Caster : public FuncImpl {
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            Src obj = get<Src>(context);
            return make_val(Dst(obj));
        }
    };
    
    template<class Src, class Dst>
    Ptr<FuncImpl> make_caster() {
        return new Caster<Src, Dst>();
    }
    
    
    struct Children : public Object {
        typedef std::map<std::string, Ptr<FuncImpl> > Map;

        Map map;
        Ptr<FuncImpl> getDirFunc();
        Ptr<FuncImpl> getChildFunc();
    };

    
} // namespace Scripting

#endif
