#include <map>
#include <string>
#include <cstdio>
#include <landscape.h>
#include <interfaces/IConfig.h>

class Config: public IConfig
{
public:
    virtual void feedArguments(const int argc, const char **argv);
    
    virtual const char *query(const char *key, const char *def =0);

private:
    std::map<std::string, std::string> data;
};
