#ifndef ICONFIG_H
#define ICONFIG_H

#include <object.h>

class IConfig : virtual public Object
{
public:
    virtual void feedArguments(int argc, const char **const argv)=0;
    
    virtual const char *query(const char *key, const char *def =0)=0;
};

#endif
