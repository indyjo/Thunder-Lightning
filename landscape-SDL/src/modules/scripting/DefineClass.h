#ifndef SCRIPTING_DEFINE_CLASS_H
#define SCRIPTING_DEFINE_CLASS_H

#include "Class.h"
#include "FuncImpl.h"

namespace Scripting {
    struct DefineFunc;
    struct DefineChildren;

    struct DefineClass {
        Class c;
        
        inline DefineClass(const char *name, const char*desc)
        : c(name, desc)
        { }
        
        inline DefineFunc beginFunc(const char *name,
                                    const char *desc="");
        
        inline DefineChildren beginChildren();
        
        inline Class endClass() {
            c.sortFuncs();
            return c;
        }
    };
    
    struct DefineFunc {
        DefineClass & c;
        Func f;

        inline DefineFunc(DefineClass & c,
                          const char *name,
                          const char*desc)
        :   c(c)
        {
            f.desc.func_name = name;
            f.desc.func_desc = desc;
        }

        template<class T>
        DefineFunc & param(const char *name, const char*desc="") {
            f.desc.param_names.push_back(name);
            f.desc.param_descs.push_back(desc);
            f.spec.param_types.push_back(Traits<T>::type);
            return *this;
        }

        template<class T>
        DefineFunc & returns() {
            f.spec.return_type = Traits<T>::type;
            return *this;
        }

        inline DefineFunc & impl(Ptr<FuncImpl> impl) {
            f.impl = impl;
            return *this;
        }

        inline DefineClass & endFunc() {
            c.c.addFunc(f);
            return c;
        }
    };

    struct DefineChildren {
        DefineClass & cls;
        Ptr<Children> children;

        inline DefineChildren(DefineClass & c)
        : cls(c), children(new Children) { }
        inline DefineChildren & child(const char *name, Ptr<FuncImpl> impl);
        inline DefineClass & endChildren();
    };
    
    inline DefineClass beginClass(const char *name,
                                  const char *desc="")
    {
        return DefineClass(name,desc);
    }

    inline DefineFunc DefineClass::beginFunc(const char *name,
                                             const char *desc)
    {
        return DefineFunc(*this, name, desc);
    }
    
    inline DefineChildren DefineClass::beginChildren() {
        return DefineChildren(*this);
    }
    
    inline DefineChildren & DefineChildren::child(
        const char *name,
        Ptr<FuncImpl> impl)
    {
        children->map.insert(std::make_pair(name,impl));
        return *this;
    }
    
    inline DefineClass & DefineChildren::endChildren()
    {
        cls.beginFunc("dir")
            .returns<Ptr<Tuple> >()
            .impl(children->getDirFunc())
        .endFunc()
        .beginFunc("child")
            .returns<Any>()
            .param<std::string>("name")
            .impl(children->getChildFunc())
        .endFunc();
        return cls;
    }

} // namespace Scripting

#endif
