#include "config.h"

void Config::feedArguments(int argc, const char **const argv)
{
    int i;
    char key[256], value[256];
    
    for (i=0; i< argc; i++) {
        if (2==sscanf(argv[i],"%255[A-Za-z0-9_]=%255s", key, value)) {
            data[key]=value;
            ls_message("  Config: %s = %s\n", key, value);
        }
    }
}

const char *Config::query(const char *key, const char *def)
{
    std::map<std::string,std::string>::const_iterator I;
    
    I=data.find(key);
    if (I!=data.end()) {
        return I->second.c_str();
    } else {
        return def;
    }
}
