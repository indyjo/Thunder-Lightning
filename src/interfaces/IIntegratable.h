#ifndef IINTEGRATABLE_H
#define IINTEGRATABLE_H

#include <object.h>

public struct IContinuousStateReader;
public struct IContinuousStateWriter;
public struct IContinuousStateFilter;

struct IIntegratable : virtual public Object {
    void getState(IContinuousStateWriter &)=0;
    void getDerivative(IContinuousStateWriter &)=0;
    void setState(IContinuousStateReader &)=0;
};

#endif