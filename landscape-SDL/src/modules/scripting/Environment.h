#ifndef SCRIPTING_ENVIRONMENT_H
#define SCRIPTING_ENVIRONMENT_H

#include <map>
#include <vector>
#include <string>
#include "Scripting.h"

namespace Scripting {
    typedef std::vector<Ptr<ValueBase> >           ObjectStack;
    typedef std::vector<std::string>               NameStack;
    typedef std::pair<ObjectStack, NameStack>      StackPair;
    typedef std::map<std::string, Ptr<ValueBase> > SymbolMap;
    
    struct Environment : public Object {
        Ptr<ValueBase> root_object;
        ObjectStack current_stack;
        NameStack name_stack;
        SymbolMap symbols;
        
        inline Environment(Ptr<ValueBase> root) {
            root_object = root;
            current_stack.push_back(root);
            name_stack.push_back("");
        }
        
        StackPair resolve(std::string path) const;
        void setCurrent(const StackPair & pair);
        
        Ptr<ValueBase> resolveChild(Ptr<ValueBase> base,
                                    std::string name) const;
        Ptr<ValueBase> resolveSymbol(std::string name) const;
    };
    
} // namespace Scripting

#endif
