#ifndef FUNC_H
#define FUNC_H

#include <string>
#include <vector>
#include <exception>
#include "Scripting.h"

namespace Scripting {

    struct FuncImpl : public Object {
        typedef const std::vector<Ptr<ValueBase> > & ArgsType;
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType params)
                throw(std::exception*) = 0;
    };

    struct FuncSpec {
        const Class * return_type;
        std::vector<const Class*> param_types;
    };
    template<class R>
    FuncSpec make_spec() {
        FuncSpec spec;
        spec.return_type=Traits<R>::type;
        return spec;
    }

    template<class R, class P1>
    FuncSpec make_spec() {
        FuncSpec spec;
        spec.return_type=Traits<R>::type;
        spec.param_types.push_back(Traits<P1>::type);
        return spec;
    }
    
    template<class R, class P1, class P2>
    FuncSpec make_spec() {
        FuncSpec spec;
        spec.return_type=Traits<R>::type;
        spec.param_types.push_back(Traits<P1>::type);
        spec.param_types.push_back(Traits<P2>::type);
        return spec;
    }
    
    template<class R, class P1, class P2, class P3>
    FuncSpec make_spec() {
        FuncSpec spec;
        spec.return_type=Traits<R>::type;
        spec.param_types.push_back(Traits<P1>::type);
        spec.param_types.push_back(Traits<P2>::type);
        spec.param_types.push_back(Traits<P3>::type);
        return spec;
    }
    
    template<class R, class P1, class P2, class P3, class P4>
    FuncSpec make_spec() {
        FuncSpec spec;
        spec.return_type=Traits<R>::type;
        spec.param_types.push_back(Traits<P1>::type);
        spec.param_types.push_back(Traits<P2>::type);
        spec.param_types.push_back(Traits<P3>::type);
        spec.param_types.push_back(Traits<P4>::type);
        return spec;
    }
    
    inline bool operator< (const FuncSpec & a, const FuncSpec & b) {
        if (a.return_type < b.return_type)
            return true;
        if (a.return_type > b.return_type)
            return false;
        int n = std::min(a.param_types.size(), b.param_types.size());
        for(int i=0; i<n; i++) {
            if (a.param_types[i] < b.param_types[i])
                return true;
            if (b.param_types[i] > b.param_types[i])
                return false;
        }
        return a.param_types.size() < b.param_types.size();
    }

    struct FuncDesc {
        std::string func_name, func_desc, return_desc;
        std::vector<std::string> param_names, param_descs;
    };

    struct Func {
        FuncSpec spec;
        FuncDesc desc;
        Ptr<FuncImpl> impl;

        Ptr<ValueBase> call(Ptr<ValueBase> context,
                            const std::vector<Ptr<ValueBase> > params)
                       throw(std::exception)
        {
            if (!impl) throw Exception("Not implemented.");
            return (*impl)(context, params);
        }
    };
}


#endif
