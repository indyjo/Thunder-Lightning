#ifndef SCRIPTING_TUPLE_H
#define SCRIPTING_TUPLE_H

#include "Scripting.h"

namespace Scripting {
    struct Tuple : public Object {
        std::vector<Ptr<ValueBase> > entries;
        inline int size() { return (int) entries.size(); }
        int append(Ptr<ValueBase> v);
        int pop();
        Ptr<ValueBase> get(int index);
        Ptr<ValueBase> copy();
        Ptr<Tuple> dir();
    };
} // namespace Scripting

#endif
