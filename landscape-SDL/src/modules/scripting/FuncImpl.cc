#include "FuncImpl.h"
#include "Tuple.h"

using namespace std;
using namespace Scripting;

namespace {
    struct DirFunc : public FuncImpl {
        Ptr<Children> children;
        
        DirFunc(Ptr<Children> c) : children(c) { }
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            typedef Children::Map::iterator Iter;
            
            Ptr<Tuple> res = new Tuple();
            
            for(Iter i=children->map.begin();
                i != children->map.end(); i++)
            {
                res->append(make_val(i->first));
            }
                     
            return make_val(res);
        }
    };

    struct ChildFunc : public FuncImpl {
        Ptr<Children> children;
        
        ChildFunc(Ptr<Children> c) : children(c) { }
        
        virtual Ptr<ValueBase>
            operator() (Ptr<ValueBase> context, ArgsType args)
            throw(std::exception*)
        {
            typedef Children::Map::iterator Iter;
            
            string name = get<string>(args[0]);
            Iter i = children->map.find(name);
            if (i == children->map.end())
                throw new Exception("Path not found.");

            vector<Ptr<ValueBase> > dummy;
            return (*(i->second))(context, dummy);
        }
    };
} // namespace

namespace Scripting {
    Ptr<FuncImpl> Children::getDirFunc() {
        return new DirFunc(this);
    }

    Ptr<FuncImpl> Children::getChildFunc() {
        return new ChildFunc(this);
    }
} // namespace Scripting
