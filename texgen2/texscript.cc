#include <cstdio>
#include <iostream>
#include "texscript.h"

using namespace std;

ostream & operator<< (ostream & os, Type t) {
    switch(t) {
        case(ANY): os << "ANY"; break;
        case(VOID): os << "VOID"; break;
        case(INT): os << "INT"; break;
        case(FLOAT): os << "FLOAT"; break;
        case(BOOL): os << "BOOL"; break;
        case(STRING): os << "STRING"; break;
        case(VECTOR): os << "VECTOR"; break;
    }
    return os;
}

Data & Data::operator= (const Data & d) {
    switch (type) {
        case STRING: delete string_ref; break;
        case VECTOR: delete vector_ref; break;
        default: break;
    }

    type = d.type;
    switch (type) {
        case INT: int_val = d.int_val; break;
        case FLOAT: float_val = d.float_val; break;
        case BOOL: bool_val = d.bool_val; break;
        case STRING: string_ref = new string(*d.string_ref); break;
        case VECTOR: vector_ref = new Vector(*d.vector_ref); break;
        default: break;
    }

    return *this;
}

ostream & operator<< (ostream & os, const Data & d) {
    switch(d.type) {
        case VOID: os << "<void>"; break;
        case INT: os << "<int:" << d.int_val << ">"; break;
        case FLOAT: os << "<float:" << d.float_val << ">"; break;
        case BOOL: os << "<bool:" << d.bool_val << ">"; break;
        case STRING: os << "<string:" << *d.string_ref << ">"; break;
        case VECTOR: {
            Vector v(*d.vector_ref);
            os << "<vector:(" << v[0]<<", "<<v[1]<<", "<<v[2] << ")>";
        } break;
        default: os << "<unknown>"; break;
    }
    return os;
}

ostream & operator<< (ostream & os, const Spec & spec) {
    os << "(";
    for (vector<Type>::size_type i=0; i<spec.in.size(); i++)
        os << " " << spec.in[i];
    os << " ) -> " << spec.out;
    return os;
}


template<class T> Literal<T>::Literal(T t) : d(t) { }
template<class T> Data Literal<T>::eval() { return d; }

template class Literal<int>;
template class Literal<float>;
template class Literal<bool>;
template class Literal<string>;
template class Literal<const char *>;
template class Literal<const Vector &>;

class Int_Float : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        return Data((int)a);
    }
};

class Int_String : public Expression {
    virtual Data eval() {
        string a=*params[0]->eval().string_ref;
        return Data(atoi(a.c_str()));
    }
};

class Int_PlusInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        return Data(a);
    }
};

class Int_MinusInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        return Data(-a);
    }
};

class Int_IntPlusInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a+b);
    }
};

class Int_IntMinusInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval();
        int b=params[1]->eval();
        return Data(a-b);
    }
};

class Int_IntMulInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a*b);
    }
};

class Int_IntDivInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a/b);
    }
};

class Int_IntModInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a%b);
    }
};

class Int_IntShiftLeftInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a<<b);
    }
};

class Int_IntShiftRightInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a>>b);
    }
};

class Bool_IntLessInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a<b);
    }
};

class Bool_IntGreaterInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a>b);
    }
};

class Bool_IntLessEqualInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a<=b);
    }
};

class Bool_IntGreaterEqualInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a>=b);
    }
};

class Bool_IntEqualsInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a==b);
    }
};

class Bool_IntNotEqualsInt : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        int b=params[1]->eval().int_val;
        return Data(a!=b);
    }
};

class Bool_NotBool : public Expression {
    virtual Data eval() {
        bool a=params[0]->eval().bool_val;
        return Data(!a);
    }
};

class Bool_BoolOrBool : public Expression {
    virtual Data eval() {
        bool a=params[0]->eval().bool_val;
        if (a) return Data(true);
        bool b=params[1]->eval().bool_val;
        return Data(b);
    }
};

class Bool_BoolAndBool : public Expression {
    virtual Data eval() {
        bool a=params[0]->eval().bool_val;
        if (!a) return Data(false);
        bool b=params[1]->eval().bool_val;
        return Data(b);
    }
};

class Float_Int : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        return Data((float)a);
    }
};

class Float_String : public Expression {
    virtual Data eval() {
        string a=*params[0]->eval().string_ref;
        return Data((float)atof(a.c_str()));
    }
};

class Float_PlusFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        return Data(a);
    }
};

class Float_MinusFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        return Data(-a);
    }
};

class Float_FloatPlusFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a+b);
    }
};

class Float_FloatMinusFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a-b);
    }
};

class Float_FloatMulFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a*b);
    }
};

class Float_FloatDivFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a/b);
    }
};

class Bool_String : public Expression {
    virtual Data eval() {
        string s=*params[0]->eval().string_ref;
        return s=="true";
    }
};

class Bool_FloatLessFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a<b);
    }
};

class Bool_FloatGreaterFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a>b);
    }
};

class Bool_FloatLessEqualFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a<=b);
    }
};

class Bool_FloatGreaterEqualFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a>=b);
    }
};

class Bool_FloatEqualsFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a==b);
    }
};

class Bool_FloatNotEqualFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        return Data(a!=b);
    }
};

class Vector_FloatFloatFloat : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        float b=params[1]->eval().float_val;
        float c=params[2]->eval().float_val;
        return Data(Vector(a,b,c));
    }
};

class Vector_VectorPlusVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        Vector b=*params[1]->eval().vector_ref;
        return Data(a+b);
    }
};

class Vector_VectorMinusVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        Vector b=*params[1]->eval().vector_ref;
        return Data(a-b);
    }
};

class Vector_VectorMulFloat : public Expression {
    virtual Data eval() {
        Vector v=*params[0]->eval().vector_ref;
        float  s=params[1]->eval().float_val;
        return Data(s*v);
    }
};

class Vector_VectorDivFloat : public Expression {
    virtual Data eval() {
        Vector v=*params[0]->eval().vector_ref;
        float  s=params[1]->eval().float_val;
        return Data((1.0/s)*v);
    }
};

class Vector_FloatMulVector : public Expression {
    virtual Data eval() {
        Vector v=*params[1]->eval().vector_ref;
        float  s=params[0]->eval().float_val;
        return Data(s*v);
    }
};

class Vector_FloatDivVector : public Expression {
    virtual Data eval() {
        Vector v=*params[1]->eval().vector_ref;
        float  s=params[0]->eval().float_val;
        return Data((1.0/s)*v);
    }
};

class Float_VectorScalarVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        Vector b=*params[1]->eval().vector_ref;
        return Data(a*b);
    }
};

class Vector_VectorCrossVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        Vector b=*params[1]->eval().vector_ref;
        return Data(a%b);
    }
};

class Float_LengthVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        return Data(a.length());
    }
};

class Vector_NormalizeVector : public Expression {
    virtual Data eval() {
        Vector a=*params[0]->eval().vector_ref;
        return Data(a.normalize());
    }
};

class String_Int : public Expression {
    virtual Data eval() {
        int a=params[0]->eval().int_val;
        char s[128];
        snprintf(s, 128, "%d", a);
        return Data(s);
    }
};

class String_Float : public Expression {
    virtual Data eval() {
        float a=params[0]->eval().float_val;
        char s[128];
        snprintf(s, 128, "%f", a);
        return Data(s);
    }
};

class String_Bool : public Expression {
    virtual Data eval() {
        bool a=params[0]->eval().bool_val;
        return Data(a?"true":"false");
    }
};

class String_Vector : public Expression {
    virtual Data eval() {
        Vector v=*params[0]->eval().vector_ref;
        char s[128];
        snprintf(s, 128, "(%f %f %f)", v[0], v[1], v[2]);
        return Data(s);
    }
};

ScriptHost::ScriptHost() {
    Spec i2i(INT, INT, INT);
    Spec i1i(INT, INT);
    Spec i2b(INT, INT, BOOL);
    Spec b2b(BOOL, BOOL, BOOL);
    Spec b1b(BOOL, BOOL);
    Spec f2f(FLOAT, FLOAT, FLOAT);
    Spec f1f(FLOAT, FLOAT);
    Spec f2b(FLOAT, FLOAT, BOOL);
    Spec f3v(FLOAT, FLOAT, FLOAT, VECTOR);
    Spec v2v(VECTOR, VECTOR, VECTOR);
    Spec fvv(FLOAT, VECTOR, VECTOR);
    Spec vfv(VECTOR, FLOAT, VECTOR);
    Spec v1f(VECTOR, FLOAT);
    Spec v1v(VECTOR, VECTOR);
    Spec v2f(VECTOR, VECTOR, FLOAT);
    Spec f1i(FLOAT, INT);
    Spec s1i(STRING, INT);
    Spec i1f(INT, FLOAT);
    Spec s1f(STRING, FLOAT);
    Spec i1s(INT, STRING);
    Spec f1s(FLOAT, STRING);
    Spec b1s(BOOL, STRING);
    Spec v1s(VECTOR, STRING);
    Spec s1b(STRING, BOOL);
    
    // Constructors and casts
    add("int",    f1i, new Factory< Int_Float >                 );
    add("int",    s1i, new Factory< Int_String >                );
    add("float",  i1f, new Factory< Float_Int >                 );
    add("float",  s1f, new Factory< Float_String >              );
    add("bool",   s1b, new Factory< Bool_String >               );
    add("vector", f3v, new Factory< Vector_FloatFloatFloat >    );
    add("string", i1s, new Factory< String_Int >                );
    add("string", f1s, new Factory< String_Float >              );
    add("string", b1s, new Factory< String_Bool >               );
    add("string", v1s, new Factory< String_Vector >             );
    
    // arithmetic operations
    add("+",  i1i,  new Factory< Int_PlusInt >                  );
    add("+",  f1f,  new Factory< Float_PlusFloat >              );
    
    add("-",  i1i,  new Factory< Int_MinusInt >                 );
    add("-",  f1f,  new Factory< Float_MinusFloat >             );
    
    add("+",  i2i,  new Factory< Int_IntPlusInt >               );
    add("+",  f2f,  new Factory< Float_FloatPlusFloat >         );
    add("+",  v2v,  new Factory< Vector_VectorPlusVector >      );
    
    add("-",  i2i,  new Factory< Int_IntMinusInt >              );
    add("-",  f2f,  new Factory< Float_FloatMinusFloat >        );
    add("-",  v2v,  new Factory< Vector_VectorMinusVector >     );
    
    add("*",  i2i,  new Factory< Int_IntMulInt >                );
    add("*",  f2f,  new Factory< Float_FloatMulFloat >          );
    add("*",  vfv,  new Factory< Vector_VectorMulFloat >        );
    add("*",  fvv,  new Factory< Vector_FloatMulVector >        );
    add("*",  v2f,  new Factory< Float_VectorScalarVector >     );
    
    add("/",  i2i,  new Factory< Int_IntDivInt >                );
    add("/",  f2f,  new Factory< Float_FloatDivFloat >          );
    add("/",  vfv,  new Factory< Vector_VectorDivFloat >        );
    add("/",  fvv,  new Factory< Vector_FloatDivVector >        );
    
    add("%",  i2i,  new Factory< Int_IntModInt >                );
    add("%",  v2v,  new Factory< Vector_VectorCrossVector >     );
    
    add("<<", i2i,  new Factory< Int_IntShiftLeftInt >          );
    add(">>", i2i,  new Factory< Int_IntShiftRightInt >         );
    
    add("<",  i2b,  new Factory< Bool_IntLessInt>               );
    add("<",  f2b,  new Factory< Bool_FloatLessFloat>           );
    
    add(">",  i2b,  new Factory< Bool_IntGreaterInt>            );
    add(">",  f2b,  new Factory< Bool_FloatGreaterFloat>        );
    
    add("<=", i2b,  new Factory< Bool_IntLessEqualInt>          );
    add("<=", f2b,  new Factory< Bool_FloatLessEqualFloat>      );
    
    add(">=", i2b,  new Factory< Bool_IntGreaterEqualInt>       );
    add(">=", f2b,  new Factory< Bool_FloatGreaterEqualFloat>   );
    
    add("==", i2b,  new Factory< Bool_IntEqualsInt>             );
    add("==", f2b,  new Factory< Bool_FloatEqualsFloat>         );
    
    add("!=", i2b,  new Factory< Bool_IntNotEqualsInt>          );
    add("!=", f2b,  new Factory< Bool_FloatNotEqualFloat>       );
    
    add("!",  b1b,  new Factory< Bool_NotBool >                 );
    add("||", b2b,  new Factory< Bool_BoolOrBool >              );
    add("&&", b2b,  new Factory< Bool_BoolAndBool >             );
    
    // Functions
    add("length", v1f, new Factory< Float_LengthVector >        );
    add("normalize", v1v, new Factory< Vector_NormalizeVector > );
}

ScriptHost::~ScriptHost() {
    for( ExprMap::iterator i=facs.begin(); i!=facs.end(); i++ ) {
        delete i->second;
    }
}

void ScriptHost::add(const string & name,
                     const Spec & spec,
                     ExpressionFactory * fac)
{
    facs[ make_pair(name, spec) ] = fac;
}

pair<Type, Expression *>
ScriptHost::query(const string & name, const Spec & spec)
{
    ExprMap::iterator i = facs.find( make_pair(name, spec) );
    if (i != facs.end()) {
        return make_pair(i->first.second.out, i->second->create());
    } else return make_pair(VOID, (Expression *) 0);
}

