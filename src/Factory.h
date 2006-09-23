// Factory.h
// 
// Defines an interface for generic object factories.
// 
// 
// IFactory, IFactory1, .. IFactory5 pass 0, 1, ... 5 parameters respectively
// to the sorce class'es constructor.
// 
// Factory* are the actual IFactory implementations.
// 
// Parameters:
//     T  the type of the destination class
//     S  the type of the source class
//     P* the types of the constructor parameters
//

template<class T> class IFactory
{
public:
    virtual Ptr<T> create()=0;
};

template<class T, class P1>
        class IFactory1
{
public:
    virtual Ptr<T> create(P1)=0;
};

template<class T, class P1, class P2>
        class IFactory2
{
public:
    virtual Ptr<T> create(P1, P2)=0;
};

template<class T, class P1, class P2, class P3>
        class IFactory3
{
public:
    virtual Ptr<T> create(P1, P2, P3)=0;
};

template<class T, class P1, class P2, class P3, class P4>
        class IFactory4
{
public:
    virtual Ptr<T> create(P1, P2, P3, P4)=0;
};

template<class T, class P1, class P2, class P3, class P4, class P5>
        class IFactory5
{
public:
    virtual Ptr<T> create(P1, P2, P3, P4, P5)=0;
};


template<class T, class S>
        class Factory : public IFactory<T>
{
public:
    virtual Ptr<T> create()
            { return new S; };
};

template<class T, class S, class P1>
        class Factory1 : public IFactory1<T, P1>
{
public:
    virtual Ptr<T> create(P1 p1)
            { return new S(p1); };
};

template<class T, class S, class P1, class P2>
        class Factory2 : public IFactory2<T, P1, P2>
{
public:
    virtual Ptr<T> create(P1 p1, P2 p2)
            { return new S(p1, p2); };
};

template<class T, class S, class P1, class P2, class P3>
        class Factory3 : public IFactory3<T, P1, P2, P3>
{
public:
    virtual Ptr<T> create(P1 p1, P2 p2, P3 p3)
            { return new S(p1, p2, p3); };
};

template<class T, class S, class P1, class P2, class P3, class P4>
        class Factory4 : public IFactory4<T, P1, P2, P3, P4>
{
public:
    virtual Ptr<T> create(P1 p1, P2 p2, P3 p3, P4 p4)
            { return new S(p1, p2, p3, p4); };
};

template<class T, class S, class P1, class P2, class P3, class P4, class P5>
        class Factory5 : public IFactory5<T, P1, P2, P3, P4, P5>
{
public:
    virtual Ptr<T> create(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
            { return new S(p1, p2, p3, p4, p5); };
};
