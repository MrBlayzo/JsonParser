#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <variant>
#include <vector>
#include <map>
#include <ostream>
#include <charconv>

class PtrOutOfBounds : public std::runtime_error
{
public:
    explicit PtrOutOfBounds()
        : std::runtime_error{"Out of bounds of string"} {};
};

class JsonParseError : public std::runtime_error
{
public:
    explicit JsonParseError()
        : std::runtime_error{"Error structure of json"} {};
};

class StringParserPtr
{
private:
    const char *cur;
    const char *end;
    const std::string data;

public:
    StringParserPtr(const std::string str)
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

class JsonParser
{
public:
    static JsonValue from_string(StringParserPtr &ptr);

private:
    static JsonValue get_list(StringParserPtr &ptr);
    static JsonValue get_dict(StringParserPtr &ptr);
    static JsonValue get_str(StringParserPtr &ptr);
    static JsonValue get_other(StringParserPtr &ptr);
    static void skip_whitespace(StringParserPtr &ptr)
    {
        while (is_skip_char(ptr.get()))
            ++ptr;
    }
    static bool is_skip_char(char c)
    {
        switch (c)
        {
        case '"':
        case ':':
        case '{':
        case '}':
        case '[':
        case ']':
        case ',':
            return false;
        default:
            return isalnum(static_cast<unsigned char>(c)) == 0;
        }
    }
};

using JsonType = std::variant<
    std::monostate,
    bool,
    int64_t,
    double,
    std::string,
    std::vector<JsonValue>,
    std::map<std::string, JsonValue>>;

struct JsonValue
{
    JsonType value;
    JsonValue() : value(std::monostate{}) {};
    JsonValue(std::nullptr_t) : value(std::monostate{}) {};
    JsonValue(bool val) : value(val) {};
    JsonValue(int64_t val) : value(val) {};
    JsonValue(int val) : value(static_cast<int64_t>(val)) {};
    JsonValue(double val) : value(val) {};
    JsonValue(std::string val) : value(std::move(val)) {};
    JsonValue(const char *val) : value(std::string(val)) {};
    JsonValue(std::vector<JsonValue> val) : value(std::move(val)) {};
    JsonValue(std::map<std::string, JsonValue> val) : value(std::move(val)) {};

    template <typename T>
    const T &get() const
    {
        return std::get<T>(value);
    };
    static JsonValue from_string(StringParserPtr &ptr);
    static JsonValue from_string(const std::string str);
    std::string to_string(const bool &new_line = false, const std::string tabs = "") const;

    bool is_null() const
    {
        return std::holds_alternative<std::monostate>(value);
    };
    bool is_bool() const
    {
        return std::holds_alternative<bool>(value);
    };
    bool is_int() const
    {
        return std::holds_alternative<int64_t>(value);
    };
    bool is_float() const
    {
        return std::holds_alternative<double>(value);
    };
    bool is_num() const
    {
        return is_int() || is_float();
    };
    bool is_str() const
    {
        return std::holds_alternative<std::string>(value);
    };
    bool is_list() const
    {
        return std::holds_alternative<std::vector<JsonValue>>(value);
    };
    bool is_dict() const
    {
        return std::holds_alternative<std::map<std::string, JsonValue>>(value);
    };

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
    JsonValue read(const std::string &file_path);
    void write(const std::string &file_path, JsonValue &data);

protected:
    std::string _read(const std::string &file_path);
    void _write(const std::string &file_path, const std::string &data);
};

class FileNotFoundError : public std::runtime_error
{
public:
    explicit FileNotFoundError(const std::string &file_path)
        : std::runtime_error{"File not found: " + std::filesystem::current_path().generic_string() + "/" + file_path} {};
};