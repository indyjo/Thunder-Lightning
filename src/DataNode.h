#ifndef DATANODE_H
#define DATANODE_H

#include <string>
#include <vector>
#include <map>
#include <modules/math/Vector.h>
#include "object.h"

class DataNodeBase {
public:
    typedef std::string Key;
    typedef std::map<Key, bool> Bools;
    typedef std::map<Key, int> Ints;
    typedef std::map<Key, float> Floats;
    typedef std::map<Key, std::string> Strings;
    typedef std::map<Key, Vector> Vectors;
        
    inline DataNodeBase() : _bools(0), _ints(0), _floats(0), _strings(0), _vectors(0) { }
    inline ~DataNodeBase() {
        delete _strings;
        delete _floats;
        delete _ints;
        delete _bools;
        delete _vectors;
    }
    
    const DataNodeBase& operator= (const DataNodeBase &);
    
    Bools &   bools() const;
    Ints &    ints() const;
    Floats &  floats() const;
    Strings & strings() const;
    Vectors & vectors() const;
    
    inline bool hasBools() const {return _bools;}
    inline bool hasInts() const {return _ints;}
    inline bool hasFloats() const {return _floats;}
    inline bool hasStrings() const {return _strings;}
    inline bool hasVectors() const {return _vectors;}
    
private:
    Bools*   _bools;
    Ints*    _ints;
    Floats*  _floats;    
    Strings* _strings;
    Vectors* _vectors;
};


class DataNode : public Object, public DataNodeBase
{
public:
    typedef std::vector<Key> Keys;
    
    inline const DataNode& operator= (const DataNode &o)
    { DataNodeBase::operator=(o); return *this; }
    
    void setBool(const Key& key, bool value);
    void setInt(const Key& key, int value);
    void setFloat(const Key& key, float value);
    void setString(const Key& key, const std::string &value);
    void setVector(const Key& key, const Vector &value);
    
    bool hasBool(const Key& key) const;
    bool hasInt(const Key& key) const;
    bool hasFloat(const Key& key) const;
    bool hasString(const Key& key) const;
    bool hasVector(const Key& key) const;
    
    bool getBool(const Key& key, bool default_value=false) const;
    int getInt(const Key& key, int default_value=0) const;
    float getFloat(const Key& key, float default_value=0.0f) const;
    std::string getString(const Key& key, const std::string &default_value="") const;
    Vector getVector(const Key& key, const Vector &default_value=Vector(0,0,0)) const;
    
    Keys keysBool() const;
    Keys keysInt() const;
    Keys keysFloat() const;
    Keys keysString() const;
    Keys keysVector() const;
};

#endif // DATANODE_H

