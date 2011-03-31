#include <map>
#include <string>
#include <cstdio>
#include <tnl.h>
#include <interfaces/IConfig.h>

class Config: public IConfig
{
public:
    virtual void feedArguments(int argc, const char **argv);
    
    virtual const char *query(const std::string& key, const char *def = 0);
    virtual float  queryFloat(const std::string& key, float def);
    virtual int    queryInt(const std::string& key, int def);
    virtual bool   queryBool(const std::string& key, bool def);
    virtual Vector queryVector(const std::string& key, Vector def);
    
    virtual void set(const std::string& key, const std::string& value);

private:
    std::map<std::string, std::string> data;
};
