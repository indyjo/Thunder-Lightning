#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cerrno>
#include "config.h"

void Config::feedArguments(int argc, const char **const argv)
{
    int i;
    char key[256], value[256];
    
    for (i=0; i< argc; i++) {
        if (2==sscanf(argv[i],"%255[A-Za-z0-9_]=%255s", key, value)) {
       		set(key,value);
        }
    }
}

void Config::set(const std::string& key, const std::string& value)
{
	data[key] = value;
    //ls_message("  Config[%p]: %s = %s\n", this, key, value);
}

const char *Config::query(const std::string& key, const char* def)
{
    std::map<std::string,std::string>::const_iterator I;
    I=data.find(key);
    if (I!=data.end()) {
        return I->second.c_str();
    } else {
    	ls_warning("Not found: key %s. Fallback to \"%s\"\n", key.c_str(), def);
        return def;
    }
}

float Config::queryFloat(const std::string& key, float def)
{
	const char *str = query(key);
	if (!str) return def;
	errno=0;
	float res = strtod(str, NULL);
	if (errno) return def;
	else return res;
}

int Config::queryInt(const std::string& key, int def)
{
	const char *str = query(key);
	if (!str) return def;
	errno=0;
	int res = strtol(str, NULL, 10);
	if (errno) return def;
	else return res;
}

bool Config::queryBool(const std::string& key, bool def)
{
	const char *str = query(key);
	if (!str) return def;
	if (0==strcmp(str, "true")) return true;
	else if (0==strcmp(str, "yes")) return true;
	else if (0==strcmp(str, "on")) return true;
	else if (0==strcmp(str, "1")) return true;
	else if (0==strcmp(str, "false")) return false;
	else if (0==strcmp(str, "no")) return false;
	else if (0==strcmp(str, "off")) return false;
	else if (0==strcmp(str, "0")) return false;
	else return def;
}

Vector Config::queryVector(const std::string& key, Vector def) {
	const char *str = query(key);
	if (!str) return def;
	std::istringstream is(str);
	Vector res;
	is >> res;
	if (is) return res;
	else return def;
}

