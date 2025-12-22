#include <fstream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <variant>
#include <vector>
#include <map>
#include <ostream>

#pragma once

using namespace std;

class PtrOutOfBounds : public runtime_error
{
public:
    explicit PtrOutOfBounds()
        : runtime_error{"Out of bounds of string"} {};
};

class JsonParseError : public runtime_error
{
public:
    explicit JsonParseError()
        : runtime_error{"Error structure of json"} {};
};

class StringParserPtr
{
private:
    const char *cur;
    const char *end;
    string data;

public:
    StringParserPtr(std::string str)
        : data(std::move(str))
    {
        cur = data.data();
        end = data.data() + data.size();
    }
    // префиксные операторы
    StringParserPtr &operator++()
    {
        if (cur >= end)
            throw PtrOutOfBounds();
        ++cur;
        return *this;
    }
    const char get()
    {
        if (cur >= end)
            throw PtrOutOfBounds();
        return *cur;
    }
};

struct JsonValue;

class JsonParser{
    public:
        static JsonValue from_string(StringParserPtr &ptr);
    private:
        static JsonValue get_list(StringParserPtr &ptr);
        static JsonValue get_dict(StringParserPtr &ptr);
        static JsonValue get_str(StringParserPtr &ptr);
        static JsonValue get_other(StringParserPtr &ptr);
};

using JsonType = variant<
    monostate,
    bool,
    int64_t,
    double,
    string,
    vector<JsonValue>,
    map<string, JsonValue>>;

struct JsonValue
{
    JsonType value;
    JsonValue() : value(monostate{}) {};
    JsonValue(nullptr_t) : value(monostate{}) {};
    JsonValue(bool val) : value(val) {};
    JsonValue(int64_t val) : value(val) {};
    JsonValue(double val) : value(val) {};
    JsonValue(string val) : value(std::move(val)) {};
    JsonValue(const char *val) : value(string(val)) {};
    JsonValue(vector<JsonValue> val) : value(std::move(val)) {};
    JsonValue(map<string, JsonValue> val) : value(std::move(val)) {};

    template <typename T>
    const T &get() const
    {
        return std::get<T>(value);
    };
    static JsonValue from_string(StringParserPtr &ptr);
    string to_string(const bool &new_line = false, const string tabs = "") const;

    bool is_null() const { return holds_alternative<monostate>(value); };
    bool is_bool() const { return holds_alternative<bool>(value); };
    bool is_int() const { return holds_alternative<int64_t>(value); };
    bool is_float() const { return holds_alternative<double>(value); };
    bool is_num() const { return is_int() || is_float(); };
    bool is_str() const { return holds_alternative<string>(value); };
    bool is_list() const { return holds_alternative<vector<JsonValue>>(value); };
    bool is_dict() const { return holds_alternative<map<string, JsonValue>>(value); };

    friend bool operator==(const JsonValue &a, const JsonValue &b)
    {
        return a.value == b.value;
    }
    friend std::ostream &operator<<(std::ostream &os, const JsonValue &json);
};

class JsonReader
{
public:
    JsonReader();
    ~JsonReader();
    JsonValue read(const string &file_path);
    void write(const string &file_path, JsonValue &data);

protected:
    string _read(const string &file_path);
    void _write(const string &file_path, const string &data);
};

class FileNotFoundError : public runtime_error
{
public:
    explicit FileNotFoundError(const string &file_path)
        : runtime_error{"File not found: " + filesystem::current_path().generic_string() + "/" + file_path} {};
};

inline bool is_skip_char(char c)
{
    switch (c)
    {
    case '"':
    case ':':
    case '{':
    case '}':
    case '[':
    case ']':
        return false;
    default:
        return isalnum(static_cast<unsigned char>(c)) == 0;
    }
}