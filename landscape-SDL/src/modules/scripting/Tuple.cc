#include "Tuple.h"


namespace {
    std::string int2string(int a) {
        char buf[32];
        snprintf(buf,32,"%d",a);
        return buf;
    }
    std::string float2string(float a) {
        char buf[32];
        snprintf(buf,32,"%f",a);
        return buf;
    }
}


namespace Scripting {
    int Tuple::append(Ptr<ValueBase> v) {
        entries.push_back(v);
        return entries.size();
    }
    int Tuple::pop() {
        entries.pop_back();
        return entries.size();
    }
    Ptr<ValueBase> Tuple::get(int index) {
        if (index<0) {
            index = entries.size()+index;
        }

        if (index<0 || index>=entries.size()) {
            throw new Exception("Index out of range");
        }

        return entries[index];
    }
    Ptr<ValueBase> Tuple::copy() {
        Ptr<Tuple> tuple = new Tuple;
        tuple->entries = entries;
        return make_val(tuple);
    }
    Ptr<Tuple> Tuple::dir() {
        Ptr<Tuple> names = new Tuple;
        names->entries.reserve(entries.size());
        for(int i=0; i<entries.size(); ++i) {
            names->append(make_val(int2string(i)));
        }
        return names;
    }
} // namespace Scripting
