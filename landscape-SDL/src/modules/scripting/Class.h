#ifndef SCRIPTING_CLASS_H
#define SCRIPTING_CLASS_H

#include <map>
#include <string>
#include "Func.h"

namespace Scripting {

struct Class {
    std::string name, desc;
    std::multimap<std::string, Func*> funcs_by_name;
    std::vector<Func*> funcs;
    
    inline Class() : name("unnamed"), desc("") { }
    inline Class(const char * name, const char * desc="")
    :   name(name), desc(desc) { }
    inline const std::string & getName() const { return name; }
    inline const std::string & getDesc() const { return desc; }
    
    void addFunc(const Func &);
    
    Func* getFunc(const std::string & name,
                  const FuncSpec & spec) const;
    
    void sortFuncs();
};

} // namespace Scripting

#endif
