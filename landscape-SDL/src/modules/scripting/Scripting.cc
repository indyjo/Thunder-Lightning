#include <string>
#include "Scripting.h"
#include "Class.h"
#include "DefineClass.h"
#include "FuncImpl.h"
#include "Tuple.h"
#include <cstdio>
#include <cmath>

using namespace Scripting;

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

namespace {

struct TupleAppendAny : public FuncImpl {
    Ptr<ValueBase>
    operator() (Ptr<ValueBase> context,
                const std::vector<Ptr<ValueBase> > & params)
    throw(std::exception*)
    {
        Ptr<Tuple> self = getPtr<Tuple>(context);
        self->append(params[0]);
        return context;
    }
};

struct TupleGetInt : public FuncImpl {
    Ptr<ValueBase>
    operator() (Ptr<ValueBase> context,
                const std::vector<Ptr<ValueBase> > & params)
    throw(std::exception*)
    {
        Ptr<Tuple> self = getPtr<Tuple>(context);
        int n = get<int>(params[0]);
        return self->get(n);
    }
};

struct TupleChild : public FuncImpl {
    Ptr<ValueBase>
    operator() (Ptr<ValueBase> context,
                const std::vector<Ptr<ValueBase> > & params)
    throw(std::exception*)
    {
        Ptr<Tuple> self = getPtr<Tuple>(context);
        std::string name = get<std::string>(params[0]);
        for (int i=0; i!=name.size(); i++)
            if (!isdigit(name[i]))
                throw new Exception("Path not found");
        int index = std::atoi(name.c_str());

        return self->get(index);
    }
};

#define FUNC0(NAME, C, RESULT)                                  \
struct NAME : public FuncImpl {                                 \
    Ptr<ValueBase>                                              \
    operator() (Ptr<ValueBase> context,                         \
                const std::vector<Ptr<ValueBase> > & params)    \
    throw(std::exception*)                                      \
    {                                                           \
        C self = get<C>(context);                               \
        return make_val(RESULT);                                \
    }                                                           \
};

#define FUNC1(NAME, C, P1, RESULT)                              \
struct NAME : public FuncImpl {                                 \
    Ptr<ValueBase>                                              \
    operator() (Ptr<ValueBase> context,                         \
                const std::vector<Ptr<ValueBase> > & params)    \
    throw(std::exception*)                                      \
    {                                                           \
        C self = get<C>(context);                               \
        P1 a = get<P1>(params[0]);                              \
        return make_val(RESULT);                                \
    }                                                           \
};

#define FUNC2(NAME, C, P1, P2, RESULT)                          \
struct NAME : public FuncImpl {                                 \
    Ptr<ValueBase>                                              \
    operator() (Ptr<ValueBase> context,                         \
                const std::vector<Ptr<ValueBase> > & params)    \
    throw(std::exception*)                                      \
    {                                                           \
        C self = get<C>(context);                               \
        P1 a = get<P1>(params[0]);                              \
        P2 b = get<P2>(params[0]);                              \
        return make_val(RESULT);                                \
    }                                                           \
};

FUNC0(Int2Float,   int, (float) self)
FUNC0(Int2String,  int, int2string(self))
FUNC0(IntNeg,      int, -self)
FUNC0(IntNot,      int, ~self)
FUNC1(IntPlusInt,  int, int, self + a)
FUNC1(IntMinusInt, int, int, self - a)
FUNC1(IntMulInt,   int, int, self * a)
FUNC1(IntDivInt,   int, int, self / a)
FUNC1(IntModInt,   int, int, self % a)
FUNC1(IntAndInt,   int, int, self&a)
FUNC1(IntOrInt,    int, int, self|a)
FUNC1(IntXorInt,   int, int, self^a)

FUNC0(Float2Int,   float, (int) self)
FUNC0(Float2String,float, float2string(self))
FUNC0(FloatNeg,    float, -self)
FUNC0(FloatFloor,  float, std::floor(self))
FUNC0(FloatCeil,   float, std::ceil(self))
FUNC0(FloatSqrt,   float, std::sqrt(self))
FUNC1(FloatPlusFloat,   float, float, self + a)
FUNC1(FloatMinusFloat,  float, float, self - a)
FUNC1(FloatMulFloat,    float, float, self * a)
FUNC1(FloatDivFloat,    float, float, self / a)
FUNC1(FloatPowFloat,    float, float, std::pow(self,a))

FUNC0(String2Int,     std::string, std::atoi(self.c_str()))
FUNC0(String2Float,   std::string, (float)std::atof(self.c_str()))
FUNC0(String2String,  std::string, self)

} // namespace


struct Types {
    Class no_type;
    Class void_type;
    Class any_type;
    Class int_type;
    Class float_type;
    Class string_type;
    Class tuple_type;
    
    Types() {
        no_type = beginClass("no_type","Not defined").endClass();
        void_type = beginClass("void","Void").endClass();
        any_type = beginClass("any","Any type").endClass();
        int_type = beginClass("int","An integer")
            .beginFunc("float","convert to float")
                .returns<float>()
                .impl(new Int2Float)
            .endFunc()
            .beginFunc("string","convert to string")
                .returns<std::string>()
                .impl(new Int2String)
            .endFunc()
            .beginFunc("neg","negate the value")
                .returns<int>()
                .impl(new IntNeg)
            .endFunc()
            .beginFunc("not","bitwise not")
                .returns<int>()
                .impl(new IntNot)
            .endFunc()
            .beginFunc("plus","add an int")
                .param<int>("x")
                .returns<int>()
                .impl(new IntPlusInt)
            .endFunc()
            .beginFunc("minus","subtract an int")
                .param<int>("x")
                .returns<int>()
                .impl(new IntMinusInt)
            .endFunc()
            .beginFunc("mul","multiply with an int")
                .param<int>("x","the value to multiply with")
                .returns<int>()
                .impl(new IntMulInt)
            .endFunc()
            .beginFunc("div","divide by an int")
                .param<int>("x","the value to divide by")
                .returns<int>()
                .impl(new IntDivInt)
            .endFunc()
            .beginFunc("mod","division rest")
                .param<int>("x","the value to divide by")
                .returns<int>()
                .impl(new IntModInt)
            .endFunc()
            .beginFunc("AND","bitwise AND")
                .param<int>("x","the value to AND with")
                .returns<int>()
                .impl(new IntAndInt)
            .endFunc()
            .beginFunc("OR","bitwise OR")
                .param<int>("x","the value to OR with")
                .returns<int>()
                .impl(new IntOrInt)
            .endFunc()
            .beginFunc("XOR","bitwise XOR")
                .param<int>("x","the value to XOR with")
                .returns<int>()
                .impl(new IntXorInt)
            .endFunc()
        .endClass();
        float_type = beginClass("float","A floating-point value")
            .beginFunc("int","convert to int")
                .returns<int>()
                .impl(new Float2Int)
            .endFunc()
            .beginFunc("string","convert to string")
                .returns<std::string>()
                .impl(new Float2String)
            .endFunc()
            .beginFunc("neg","negate the value")
                .returns<float>()
                .impl(new FloatNeg)
            .endFunc()
            .beginFunc("floor","returns the next-lower integer")
                .returns<float>()
                .impl(new FloatFloor)
            .endFunc()
            .beginFunc("ceil","returns the next-higher integer")
                .returns<float>()
                .impl(new FloatCeil)
            .endFunc()
            .beginFunc("sqr","returns the square root")
                .returns<float>()
                .impl(new FloatSqrt)
            .endFunc()
            .beginFunc("plus","add a float")
                .param<float>("x")
                .returns<float>()
                .impl(new FloatPlusFloat)
            .endFunc()
            .beginFunc("minus","subtract a float")
                .param<float>("x")
                .returns<float>()
                .impl(new FloatMinusFloat)
            .endFunc()
            .beginFunc("mul","multiply with an float")
                .param<float>("x","the value to multiply with")
                .returns<float>()
                .impl(new FloatMulFloat)
            .endFunc()
            .beginFunc("div","divide by a float")
                .param<float>("x","the value to divide by")
                .returns<float>()
                .impl(new FloatDivFloat)
            .endFunc()
        .endClass();
        string_type = beginClass("string","A character string")
            .beginFunc("float","convert to float")
                .returns<float>()
                .impl(new String2Float)
            .endFunc()
            .beginFunc("int", "convert to int")
                .returns<int>()
                .impl(new String2Int)
            .endFunc()
            .beginFunc("string", "returns the string")
                .returns<std::string>()
                .impl(new String2String)
            .endFunc()
        .endClass();
        tuple_type = beginClass("tuple","A list of values")
            .beginFunc("append","appends an entry at the end")
                .param<Any>("x","the value to append")
                .returns<int>()
                .impl(new TupleAppendAny)
            .endFunc()
            .beginFunc("size","returns the number of entries")
                .returns<int>()
                .impl(make_impl(&Tuple::size))
            .endFunc()
            .beginFunc("pop","Removes the last entry")
                .returns<int>()
                .impl(make_impl(&Tuple::pop))
            .endFunc()
            .beginFunc("get","Gets the n-th value")
                .param<int>("n","the index to get")
                .returns<Any>()
                .impl(new TupleGetInt)
            .endFunc()
            .beginFunc("dir","Lists the entries as children")
                .returns<Ptr<Tuple> >()
                .impl(make_impl(&Tuple::dir))
            .endFunc()
            .beginFunc("child","gets a child by name")
                .param<std::string>("name","the child's name")
                .returns<Any>()
                .impl(new TupleChild)
            .endFunc()
        .endClass();
    }
} types;

namespace Scripting {
    const Class * UndefinedTraits::query_type = & types.no_type;
}

DEFINE_TRAITS(Any, types.any_type)
DEFINE_TRAITS(int, types.int_type)
DEFINE_TRAITS(float, types.float_type)
DEFINE_TRAITS(std::string, types.string_type)
DEFINE_TRAITS(Ptr<Tuple>, types.tuple_type)


