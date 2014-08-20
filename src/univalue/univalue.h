// Copyright 2014 BitPay Inc.
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UNIVALUE_UNIVALUE_H
#define BITCOIN_UNIVALUE_UNIVALUE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>

#include <sstream>        // .get_int64()
#include <utility>        // std::pair
#include <stdlib.h>       // atoi(), atof()   TODO: remove

class UniValue {
public:
    enum VType { VNULL, VOBJ, VARR, VSTR, VNUM, VBOOL, };

    UniValue() { typ = VNULL; }
    UniValue(UniValue::VType initialType, const std::string& initialStr = "") {
        typ = initialType;
        val = initialStr;
    }
    UniValue(uint64_t val_) {
        setInt(val_);
    }
    UniValue(int64_t val_) {
        setInt(val_);
    }
    UniValue(int val_) {
        setInt(val_);
    }
    UniValue(double val_) {
        setFloat(val_);
    }
    UniValue(const std::string& val_) {
        setStr(val_);
    }
    UniValue(const char *val_) {
        std::string s(val_);
        setStr(s);
    }
    ~UniValue() {}

    void clear();

    bool setNull();
    bool setBool(bool val);
    bool setNumStr(const std::string& val);
    bool setInt(uint64_t val);
    bool setInt(int64_t val);
    bool setInt(int val) { return setInt((int64_t)val); }
    bool setFloat(double val);
    bool setStr(const std::string& val);
    bool setArray();
    bool setObject();

    enum VType getType() const { return typ; }
    std::string getValStr() const { return val; }
    bool empty() const { return (values.size() == 0); }

    size_t size() const { return values.size(); }

    bool getBool() const { return isTrue(); }
    bool checkObject(const std::map<std::string,UniValue::VType>& memberTypes);
    const UniValue& operator[](const std::string& key) const;
    const UniValue& operator[](unsigned int index) const;
    bool exists(const std::string& key) const { return (findKey(key) >= 0); }

    bool isNull() const { return (typ == VNULL); }
    bool isTrue() const { return (typ == VBOOL) && (val == "1"); }
    bool isFalse() const { return (!isTrue()); }
    bool isBool() const { return (typ == VBOOL); }
    bool isStr() const { return (typ == VSTR); }
    bool isNum() const { return (typ == VNUM); }
    bool isArray() const { return (typ == VARR); }
    bool isObject() const { return (typ == VOBJ); }

    bool push_back(const UniValue& val);
    bool push_back(const std::string& val_) {
        UniValue tmpVal(VSTR, val_);
        return push_back(tmpVal);
    }
    bool push_back(const char *val_) {
        std::string s(val_);
        return push_back(s);
    }
    bool push_backV(const std::vector<UniValue>& vec);

    bool pushKV(const std::string& key, const UniValue& val);
    bool pushKV(const std::string& key, const std::string& val) {
        UniValue tmpVal(VSTR, val);
        return pushKV(key, tmpVal);
    }
    bool pushKV(const std::string& key, const char *val_) {
        std::string val(val_);
        return pushKV(key, val);
    }
    bool pushKV(const std::string& key, int64_t val) {
        UniValue tmpVal(val);
        return pushKV(key, tmpVal);
    }
    bool pushKV(const std::string& key, uint64_t val) {
        UniValue tmpVal(val);
        return pushKV(key, tmpVal);
    }
    bool pushKV(const std::string& key, int val) {
        UniValue tmpVal((int64_t)val);
        return pushKV(key, tmpVal);
    }
    bool pushKV(const std::string& key, double val) {
        UniValue tmpVal(val);
        return pushKV(key, tmpVal);
    }
    bool pushKVs(const UniValue& obj);

    std::string write(unsigned int prettyIndent = 0,
                      unsigned int indentLevel = 0) const;

    bool read(const char *raw);
    bool read(const std::string& rawStr) {
        return read(rawStr.c_str());
    }

private:
    UniValue::VType typ;
    std::string val;                       // numbers are stored as C++ strings
    std::vector<std::string> keys;
    std::vector<UniValue> values;

    int findKey(const std::string& key) const;
    void writeArray(unsigned int prettyIndent, unsigned int indentLevel, std::string& s) const;
    void writeObject(unsigned int prettyIndent, unsigned int indentLevel, std::string& s) const;

public:
    //
    // The following were added for compatibility with json_spirit.
    // Most duplicate other methods, and should be removed.
    //
    std::vector<std::string> getKeys() const { return keys; }
    std::vector<UniValue> getValues() const { return values; }
    bool get_bool() const { return getBool(); }
    std::string get_str() const { return getValStr(); }
    int get_int() const { return atoi(getValStr().c_str()); }
    double get_real() const { return atof(getValStr().c_str()); }
    const UniValue& get_obj() const { return *this; }
    const UniValue& get_array() const { return *this; }
    enum VType type() const { return getType(); }
    bool push_back(std::pair<std::string,UniValue> pear) {
        return pushKV(pear.first, pear.second);
    }
    int64_t get_int64() const {
        int64_t ret;
        std::istringstream(getValStr()) >> ret;
        return ret;
    }
};

//
// The following were added for compatibility with json_spirit.
// Most duplicate other methods, and should be removed.
//
static inline std::pair<std::string,UniValue> Pair(const char *cKey, const char *cVal)
{
    std::string key(cKey);
    UniValue uVal(cVal);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, std::string strVal)
{
    std::string key(cKey);
    UniValue uVal(strVal);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, uint64_t u64Val)
{
    std::string key(cKey);
    UniValue uVal(u64Val);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, int64_t i64Val)
{
    std::string key(cKey);
    UniValue uVal(i64Val);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, int iVal)
{
    std::string key(cKey);
    UniValue uVal(iVal);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, double dVal)
{
    std::string key(cKey);
    UniValue uVal(dVal);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(const char *cKey, const UniValue& uVal)
{
    std::string key(cKey);
    return std::make_pair(key, uVal);
}

static inline std::pair<std::string,UniValue> Pair(std::string key, const UniValue& uVal)
{
    return std::make_pair(key, uVal);
}

enum jtokentype {
    JTOK_ERR        = -1,
    JTOK_NONE       = 0,                           // eof
    JTOK_OBJ_OPEN,
    JTOK_OBJ_CLOSE,
    JTOK_ARR_OPEN,
    JTOK_ARR_CLOSE,
    JTOK_COLON,
    JTOK_COMMA,
    JTOK_KW_NULL,
    JTOK_KW_TRUE,
    JTOK_KW_FALSE,
    JTOK_NUMBER,
    JTOK_STRING,
};

extern enum jtokentype getJsonToken(std::string& tokenVal,
                                    unsigned int& consumed, const char *raw);
extern const char *uvTypeName(UniValue::VType t);

extern const UniValue NullUniValue;
#endif // BITCOIN_UNIVALUE_UNIVALUE_H

