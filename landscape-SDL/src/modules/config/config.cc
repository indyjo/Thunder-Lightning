#include <cstdlib>
#include <cstring>
#include <sstream>
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

void Config::set(const char *key, const char *value)
{
	data[key] = value;
    ls_message("  Config: %s = %s\n", key, value);
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

float Config::queryFloat(const char *key, float def)
{
	const char *str = query(key);
	if (!str) return def;
	errno=0;
	float res = strtof(str, NULL);
	if (errno) return def;
	else return res;
}

int Config::queryInt(const char *key, int def)
{
	const char *str = query(key);
	if (!str) return def;
	errno=0;
	int res = strtol(str, NULL, 10);
	if (errno) return def;
	else return res;
}

bool Config::queryBool(const char *key, bool def)
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

Vector Config::queryVector(const char *key, Vector def) {
	const char *str = query(key);
	if (!str) return def;
	std::istringstream is(str);
	Vector res;
	is >> res;
	if (is) return res;
	else return def;
}

