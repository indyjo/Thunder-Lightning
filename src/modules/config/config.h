#include <map>
#include <string>
#include <cstdio>
#include <tnl.h>
#include <interfaces/IConfig.h>

class Config: public IConfig
{
public:
    virtual void feedArguments(int argc, const char **argv);
    
    virtual const char *query(const char *key, const char *def =0);
    virtual float  queryFloat(const char *key, float def);
    virtual int    queryInt(const char *key, int def);
    virtual bool   queryBool(const char *key, bool def);
    virtual Vector queryVector(const char *key, Vector def);
    
    virtual void set(const char *key, const char *value);

private:
    std::map<std::string, std::string> data;
};
