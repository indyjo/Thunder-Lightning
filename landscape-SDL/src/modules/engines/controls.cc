#include "controls.h"

using namespace std;

bool Controls::getBool(const string & name) const throw(NoSuchControl) {
    map<string,bool>::const_iterator i = bools.find(name);
    if ( i!= bools.end() ) {
        return i->second;
    } else {
        throw NoSuchControl();
    }
}
void Controls::setBool(const string & name, bool val) { bools[name] = val; }

int Controls::getInt(const string & name) const throw(NoSuchControl) {
    map<string,int>::const_iterator i = ints.find(name);
    if ( i!= ints.end() ) {
        return i->second;
    } else {
        throw NoSuchControl();
    }
}
void Controls::setInt(const string & name, int val) { ints[name] = val; }

float Controls::getFloat(const string & name) const throw(NoSuchControl) {
    map<string,float>::const_iterator i = floats.find(name);
    if ( i!= floats.end() ) {
        return i->second;
    } else {
        throw NoSuchControl();
    }
}
void Controls::setFloat(const string & name, float val) { floats[name] = val; }
