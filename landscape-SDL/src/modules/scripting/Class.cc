#include <algorithm>
#include "Class.h"

namespace {
    using namespace Scripting;
    using namespace std;
    struct FuncSort {
        bool operator() (const Func *a, const Func *b) {
            if (a->desc.func_name < b->desc.func_name)
                return true;
            if (a->desc.func_name > b->desc.func_name)
                return false;
            int n = a->spec.param_types.size();
            int m = b->spec.param_types.size();
            if (n < m)
                return true;
            if (n > m)
                return false;
            for(int i=0; i<n; ++i) {
                const Class *ca = a->spec.param_types[i];
                const Class *cb = b->spec.param_types[i];
                if (ca<cb)
                    return true;
                if (ca>cb)
                    return false;
            }
            return a->spec.return_type < b->spec.return_type;
        }
    };
    
    struct FuncsByName {
        bool operator() (const Func *a, const Func *b) {
            return a->desc.func_name < b->desc.func_name;
        }
    };
    
    struct FuncsByParamNum {
        bool operator() (const Func *a, const Func *b) {
            return a->spec.param_types.size()
                 < b->spec.param_types.size();
        }
    };    
    
    struct FuncsByParam {
        int param;
        FuncsByParam(int p) : param(p) { }
        bool operator() (const Func *a, const Func *b) {
            return a->spec.param_types[param]
                 < b->spec.param_types[param];
        }
    };    

    struct FuncsByReturn {
        bool operator() (const Func *a, const Func *b) {
            return a->spec.return_type
                 < b->spec.return_type;
        }
    };    

    template<class Iter>
    pair<Iter,Iter>
    getFuncsWithName(
        const string & name,
        Iter begin,
        Iter end)
    {
        Func f;
        f.desc.func_name = name;
        return equal_range(begin,end,&f,FuncsByName());
    }
    
    template<class Iter>
    pair<Iter,Iter>
    getFuncsWithParamNum( int params, Iter begin, Iter end)
    {
        Func f;
        f.spec.param_types.assign(params, Traits<Any>::type);
        return equal_range(begin,end,&f,FuncsByParamNum());
    }

    template<class Iter>
    pair<Iter,Iter>
    getFuncsWithParam(
        int param,
        const Class *type,
        Iter begin,
        Iter end)
    {
        Func f;
        f.spec.param_types.assign(param+1, type);
        return equal_range(begin,end,&f,FuncsByParam(param));
    }
    
    template<class Iter>
    pair<Iter,Iter>
    getFuncsWithReturn(
        const Class *type,
        Iter begin,
        Iter end)
    {
        Func f;
        f.spec.return_type = type;
        return equal_range(begin,end,&f,FuncsByReturn());
    }

    
    template<class Iter>
    Func *
    findFuncMatching(
        const FuncSpec & spec,
        Iter begin,
        Iter end,
        int param=0)
    {
        if (begin == end) return 0;
        if (param == spec.param_types.size()) {
            if (spec.return_type == Traits<Any>::type) {
                return *begin;
            } else {
                pair<Iter,Iter> pair =
                    getFuncsWithReturn(spec.return_type,
                                       begin, end);
                if (pair.first==pair.second) {
                    return 0;
                } else {
                    return *pair.first;
                }
            }
        }
        if (spec.param_types[param] == Traits<Any>::type) {
            return findFuncMatching(spec,begin,end,param+1);
        }
        pair<Iter,Iter> pair;
        pair = getFuncsWithParam(param, spec.param_types[param],
                                 begin, end);
        Func *f= findFuncMatching(spec,
                                  pair.first, pair.second,
                                  param+1);
        if (f) return f;
        
        pair = getFuncsWithParam(param, Traits<Any>::type,
                                 begin, end);
        return findFuncMatching(spec,pair.first, pair.second,
                                param+1);
    }
}

namespace Scripting {
    void Class::addFunc(const Func & func) {
        Func * f = new Func(func);
        funcs.push_back(f);
    }
    
    void Class::sortFuncs() {
        std::sort(funcs.begin(), funcs.end(), FuncSort());
    }
    
    Func* Class::getFunc(const std::string & name,
                         const FuncSpec & spec) const
    {
        typedef std::vector<Func*> FuncVector;
        typedef std::vector<Func*>::const_iterator Iter;
        typedef std::pair<Iter,Iter> IterPair;
        
        IterPair pair = getFuncsWithName(
            name,
            funcs.begin(), funcs.end());
        int n = spec.param_types.size();
        pair = getFuncsWithParamNum(n, pair.first, pair.second);
        if (pair.first == pair.second) return 0;
        
        return findFuncMatching(spec, pair.first, pair.second);
    }
}

