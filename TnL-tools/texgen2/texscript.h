#ifndef TEXSCRIPT_H
#define TEXSCRIPT_H

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include "Vector.h"

enum Type { ANY, VOID, INT, FLOAT, BOOL, STRING, VECTOR };
std::ostream & operator<< (std::ostream & os, Type t);

struct Data {
    Type type;
    union {
        int     int_val;
        float   float_val;
        bool    bool_val;
        std::string *string_ref;
        Vector *vector_ref;
        
    };
    
    inline Data() : type(VOID) { }
    inline Data(int i) : type(INT), int_val(i) { }
    inline Data(float f) : type(FLOAT), float_val(f) { }
    inline Data(bool b) : type(BOOL), bool_val(b) { }
    inline Data(const char * s) : type(STRING), string_ref(new std::string(s)) { }
    inline Data(const std::string & s) : type(STRING), string_ref(new std::string(s)) { }
    inline Data(const Vector & v) : type(VECTOR), vector_ref(new Vector(v)) { }
    
    Data & operator= (const Data & d);    
    template<class T> inline Data & operator= (T t) {
        return (*this) = Data(t);
    }
    
    inline operator int () { return int_val; }
    inline operator float () { return float_val; }
    inline operator bool () { return bool_val; }
    inline operator const std::string & () { return *string_ref; }
    inline operator const Vector & () { return *vector_ref; }
};

std::ostream & operator<< (std::ostream & os, const Data & d);

struct Spec {
    std::vector<Type> in;
    Type out;
    
    inline Spec(Type out) : out(out) { }
    inline Spec(Type in0, Type out) : out(out) {
        in.push_back(in0);
    }
    inline Spec(Type in0, Type in1, Type out) : out(out) {
        in.push_back(in0);
        in.push_back(in1);
    }
    inline Spec(Type in0, Type in1,
                Type in2, Type out) : out(out) {
        in.push_back(in0);
        in.push_back(in1);
        in.push_back(in2);
    }
    inline Spec(Type in0, Type in1, Type in2,
                Type in3, Type out) : out(out) {
        in.push_back(in0);
        in.push_back(in1);
        in.push_back(in2);
        in.push_back(in3);
    }
    inline Spec(int insize, const Type *in, Type out)
    : out(out) {
        for(int i=0; i<insize; i++) this->in.push_back(in[i]);
    }
    inline ~Spec() {
    }
    
    // Needed when used as map key. Lexical order
    // We don't compare the out types.
    inline bool operator< (const Spec & s) const {
        int n=std::min(in.size(), s.in.size());
        for(int i=0; i<n; i++) {
            if (in[i] < s.in[i]) return true;
            else if (in[i] > s.in[i]) return false;
        }
        return in.size() < s.in.size();
    }
};

std::ostream & operator<< (std::ostream & os, const Spec & spec);

class Expression {
public: 
    virtual ~Expression() {
        for(ParamIter i=params.begin(); i!=params.end(); i++)
            delete *i;
    }
    
    inline Expression * param(Expression * e) {
        params.push_back(e);
        return this;
    }
    virtual Data eval() = 0;
    
protected:
    typedef std::vector<Expression *> ParamVector;
    typedef ParamVector::iterator ParamIter;
    ParamVector params;
};

class ExpressionFactory {
public:
    virtual Expression * create() = 0;
    virtual ~ExpressionFactory() { }
};

template<class T> class Factory : public ExpressionFactory {
public:
    Expression * create() { return new T; }
};

class ScriptHost {
public:
    ScriptHost();
    ~ScriptHost();
    void add(const std::string & name, const Spec & spec, ExpressionFactory * fac);
    std::pair<Type, Expression *> query(const std::string & name, const Spec & spec);
private:
    typedef std::map<std::pair<std::string, Spec>, ExpressionFactory *> ExprMap;
    ExprMap facs;
};

template<class T> class Literal : public Expression {
public:
    Literal(T t);
    virtual Data eval();
protected:
    Data d;
};

template<class T> inline Expression * literal(T t) {return new Literal<T>(t);}

#endif

