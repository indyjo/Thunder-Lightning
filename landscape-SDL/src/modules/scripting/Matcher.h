#ifndef SCRIPTING_MATCHER_H
#define SCRIPTING_MATCHER_H

#include "Tokenizer.h"

#define NEXT_TOKEN \
        if (!matched) return *this; \
        if (!tok.nextToken()) {     \
            matched = false;        \
            return *this;           \
        }
#define TEST_TYPE(func) \
        if (!tok.func()) {          \
            matched = false;        \
            return *this;           \
        }
#define EXTRACT(func)             \
        target = tok.func();

namespace Scripting {

struct Matcher {
    Tokenizer & tok;
    bool matched;

    Matcher(Tokenizer & tok) : tok(tok), matched(true) { }

    inline Matcher & _int(int & target) {
        NEXT_TOKEN
        TEST_TYPE(isInteger)
        EXTRACT(getInteger)
        return *this;
    }
    inline Matcher & _float(float & target) {
        NEXT_TOKEN
        TEST_TYPE(isFloat)
        EXTRACT(getFloat)
        return *this;
    }
    inline Matcher & _string(std::string & target) {
        NEXT_TOKEN
        TEST_TYPE(isString)
        EXTRACT(getString)
        return *this;
    }
    inline Matcher & _path(std::string & target) {
        NEXT_TOKEN
        TEST_TYPE(isPath)
        EXTRACT(getPath)
        return *this;
    }
    inline Matcher & _identifier(std::string & target) {
        NEXT_TOKEN
        TEST_TYPE(isIdentifier)
        EXTRACT(getIdentifier)
        return *this;
    }
    inline Matcher & _(const char *lit) {
        NEXT_TOKEN
        if (!tok.is(lit)) {
            matched = false;
        }
        return *this;
    }
    inline Matcher & _end() {
        if (!matched) return *this;
        if (tok.nextToken()) matched = false;
        return *this;
    }
    inline operator bool () { return matched; }
    inline bool operator! () { return !matched; }
};

} // namespace Scripting


#endif
