#include "Environment.h"
#include "Class.h"

#include <cctype>

using namespace std;

namespace Scripting {

StackPair Environment::resolve(string path) const {
    ObjectStack cur_stack;
    NameStack cur_names;
    
    if (path.size()==0)
        return make_pair(current_stack, name_stack);

    int index=0;
    if (path[0]=='/') {
        cur_stack.push_back(root_object);
        cur_names.push_back("");
        index++;
    } else {
        cur_stack = current_stack;
        cur_names = name_stack;
    }

    while(index < path.size()) {
        if (path[index] == '.') {
            if (index+1==path.size()) {
                // we are at end of path
                break;
            }
            if (path[index+1]=='.') {
                if (cur_stack.size()==1)
                    throw new Exception("Illegal path.");
                cur_stack.pop_back();
                cur_names.pop_back();
                ++index;
            } else if (path[index+1]=='/') {
                index+=2;
            } else {
                throw new Exception("Illegal path.");
            }
        } else if(path[index]=='/') {
            ++index;
        } else if(isalnum(path[index]) || path[index]=='_' || path[index]=='-') {
            int i=index+1;
            while(i < path.size()) {
                char c = path[i];
                if (!isalnum(c) && c!='_' && c!='-')
                    break;
                ++i;
            }
            string name = path.substr(index, i-index);
            Ptr<ValueBase> child;
            if(index == 0) {
                child = resolveSymbol(name);
                if (child) {
                    cur_stack.clear();
                } else {
                    child = resolveChild(cur_stack.back(), name);
                }
            } else {
                child = resolveChild(cur_stack.back(), name);
            }

            if (child) {
                cur_stack.push_back(child);
                cur_names.push_back(name);
            } else {
                throw new Exception("Path not found");
            }

            index = i;
        } else {
            throw new Exception("Illegal path");
        }
    }
    return make_pair(cur_stack, cur_names);
}

void Environment::setCurrent(const StackPair & pair) {
    current_stack = pair.first;
    name_stack = pair.second;
}

Ptr<ValueBase>
Environment::resolveChild(Ptr<ValueBase> base, string name) const
{
    FuncSpec spec = make_spec<Any, string>();
    Func* f = base->getType()->getFunc("child", spec);
    if (!f) {
        return 0;
    }
    vector<Ptr<ValueBase> > params;
    params.push_back(make_val(name));
    Ptr<ValueBase> result;
    result = (*f->impl)(base,params);
    return result;
}

Ptr<ValueBase> Environment::resolveSymbol(string name) const
{
    SymbolMap::const_iterator iter = symbols.find(name);
    if (iter == symbols.end()) {
        return 0;
    } else {
        return iter->second;
    }
}


} // namespace Scripting
