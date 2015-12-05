// for ls_message
#include "tnl.h"

#include "DataNode.h"

DataNodeBase::Bools &   DataNodeBase::bools() const {
    if (!hasBools()) const_cast<DataNodeBase*>(this)->_bools = new Bools;
    return *_bools;
}

DataNodeBase::Ints &    DataNodeBase::ints() const {
    if (!hasInts()) const_cast<DataNodeBase*>(this)->_ints = new Ints;
    return *_ints;
}

DataNodeBase::Floats &  DataNodeBase::floats() const {
    if (!hasFloats()) const_cast<DataNodeBase*>(this)->_floats = new Floats;
    return *_floats;
}

DataNodeBase::Strings & DataNodeBase::strings() const {
    if (!hasStrings()) const_cast<DataNodeBase*>(this)->_strings = new Strings;
    return *_strings;
}

DataNodeBase::Vectors & DataNodeBase::vectors() const {
    if (!hasVectors()) const_cast<DataNodeBase*>(this)->_vectors = new Vectors;
    return *_vectors;
}

void DataNode::setBool(const DataNode::Key& key, bool value)
{ bools()[key] = value; }
 
void DataNode::setInt(const DataNode::Key& key, int value)
{ ints()[key] = value; }

void DataNode::setFloat(const DataNode::Key& key, float value)
{ floats()[key] = value; }

void DataNode::setString(const DataNode::Key& key, const std::string &value)
{ strings()[key] = value; }

void DataNode::setVector(const DataNode::Key& key, const Vector &value)
{ vectors()[key] = value; }


bool DataNode::hasBool(const DataNode::Key& key) const
{ return hasBools() && bools().find(key) != bools().end(); }

bool DataNode::hasInt(const DataNode::Key& key) const
{ return hasInts() && ints().find(key) != ints().end(); }

bool DataNode::hasFloat(const DataNode::Key& key) const
{ return hasFloats() && floats().find(key) != floats().end(); }

bool DataNode::hasString(const DataNode::Key& key) const
{ return hasStrings() && strings().find(key) != strings().end(); }

bool DataNode::hasVector(const DataNode::Key& key) const
{ return hasVectors() && vectors().find(key) != vectors().end(); }


bool DataNode::getBool(const DataNode::Key& key, bool default_value) const {
    if (!hasBools()) return default_value;
    Bools::iterator iter = bools().find(key);
    return (iter != bools().end()) ? iter->second : default_value;
}

int DataNode::getInt(const DataNode::Key& key, int default_value) const {
    if (!hasInts()) return default_value;
    Ints::iterator iter = ints().find(key);
    return (iter != ints().end()) ? iter->second : default_value;
}

float DataNode::getFloat(const DataNode::Key& key, float default_value) const {
    if (!hasFloats()) return default_value;
    Floats::iterator iter = floats().find(key);
    return (iter != floats().end()) ? iter->second : default_value;
}

std::string DataNode::getString(const DataNode::Key& key, const std::string &default_value) const {
    if (!hasStrings()) return default_value;
    Strings::iterator iter = strings().find(key);
    return (iter != strings().end()) ? iter->second : default_value;
}

Vector DataNode::getVector(const DataNode::Key& key, const Vector &default_value) const {
    if (!hasVectors()) return default_value;
    Vectors::iterator iter = vectors().find(key);
    return (iter != vectors().end()) ? iter->second : default_value;
}

/*
DataNode::Keys DataNode::keysBool();
DataNode::Keys DataNode::keysInt();
DataNode::Keys DataNode::keysFloat();
DataNode::Keys DataNode::keysString();
*/

