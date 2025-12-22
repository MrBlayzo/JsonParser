#include "json.h"
#include <iostream>

using namespace std;

JsonValue JsonValue::from_string(StringParserPtr &ptr)
{
    return JsonParser::from_string(ptr);
}
JsonValue JsonParser::from_string(StringParserPtr &ptr){
    while (is_skip_char(ptr.get()))
        ++ptr;
    const char op = ptr.get();
    if (op == '[')
        return get_list(ptr);
    else if (op == '{')
        return get_dict(ptr);
    else if (ptr.get() == '"')
        return get_str(ptr);
    else
        return get_other(ptr);
};

JsonValue JsonParser::get_list(StringParserPtr &ptr){
    vector<JsonValue> data;
    ++ptr;
    while (ptr.get() != ']')
    {
        data.push_back(from_string(ptr));
        while (is_skip_char(ptr.get()))
            ++ptr;
    }
    ++ptr;
    return JsonValue(data);
}

JsonValue JsonParser::get_dict(StringParserPtr &ptr){
    map<string, JsonValue> data;
    ++ptr;
    string name;
    JsonValue param;
    while (ptr.get() != '}')
    {
        if (ptr.get() == ',')
        {
            name.clear();
            ++ptr;
        }
        else if (ptr.get() == ':')
        {
            ++ptr;
            param = from_string(ptr);
            data[name] = param;
        }
        else if (ptr.get() == '"')
        {
            ++ptr;
            name = string(1, ptr.get());
            ++ptr;
            while (ptr.get() != '\"')
            {
                name += ptr.get();
                ++ptr;
            }
            ++ptr;
        }
        else if (!is_skip_char(ptr.get()))
            throw JsonParseError();
        else
        {
            ++ptr;
        }
    }
    ++ptr;
    return JsonValue(data);
}

JsonValue JsonParser::get_str(StringParserPtr &ptr){
    ++ptr;  // пропускаем открывающую "
    string str;
    while (true) {
        char c = ptr.get();
        if (c == '"') {
            ++ptr;  // пропускаем закрывающую "
            return JsonValue(std::move(str));
        }
        if (c == '\\') {
            ++ptr;  // пропускаем '\'
            c = ptr.get();
            switch (c) {
                case '"':  c = '"';  break;
                case '\\': c = '\\'; break;
                case '/':  c = '/';  break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                case 't':  c = '\t'; break;
                case 'u': {
                    // Простой вариант: игнорируем \uXXXX (можно расширить позже)
                    // Пропускаем 'u' + 4 hex-цифры
                    for (int i = 0; i < 4; ++i) ++ptr;
                    c = '?'; // заглушка — или реализуйте конвертацию Unicode
                    break;
                }
                default:
                    // Нестандартный эскейп — оставляем как есть (или бросить ошибку)
                    break;
            }
        }
        str += c;
        ++ptr;
    }
}

JsonValue JsonParser::get_other(StringParserPtr &ptr){
    string str = string(1, ptr.get());
    ++ptr;
    while (ptr.get() != ',' &&
           ptr.get() != ' ' &&
           ptr.get() != '\n' &&
           ptr.get() != '\t' &&
           ptr.get() != ']' &&
           ptr.get() != '}')
    {
        str += ptr.get();
        ++ptr;
    }
    if (str == "false")
    {
        return JsonValue(false);
    }
    if (str == "true")
    {
        return JsonValue(true);
    }
    if (str == "null")
    {
        return JsonValue();
    }
    size_t pos = 0;
    try {
        if (str.find('.') != string::npos || str.find('e') != string::npos || str.find('E') != string::npos) {
            double d = std::stod(str, &pos);
            if (pos == str.size()) return JsonValue(d);
        } else {
            int64_t i = std::stoll(str, &pos);
            if (pos == str.size()) return JsonValue(i);
        }
    } catch (...) {}
    throw JsonParseError();
}

std::ostream &operator<<(std::ostream &os, const JsonValue &json)
{
    return os << json.to_string();
}

string JsonValue::to_string(const bool &new_line, const string tabs) const
{
    string base;
    if (is_null())
        base = "null";
    else if (is_bool())
        base = (get<bool>() ? "true" : "false");
    else if (is_num())
        base = is_int() ? std::to_string(get<int64_t>()) : std::to_string(get<double>());
    else if (is_str())
        base = "\"" + get<string>() + "\"";
    else if (is_list())
    {
        base = "[\n";
        bool first = true;
        for (const auto &obj : get<vector<JsonValue>>())
        {
            if (!first)
                base += ",\n";
            base += tabs + obj.to_string(true, tabs);
            first = false;
        }
        base += "\n" + tabs + "]";
    }
    else if (is_dict())
    {
        base = "{\n";
        bool first = true;
        for (const auto &[name, val] : get<map<string, JsonValue>>())
        {
            if (!first)
                base += ",\n";
            base += tabs + "\t" + "\"" + name + "\"" + ": " + val.to_string(false, tabs + "\t");
            first = false;
        }
        base += "\n" + tabs + "}";
    }
    else
    {
        base = "";
    }
    return new_line ? tabs + base : base;
};

JsonReader::JsonReader() {};
JsonReader::~JsonReader() {};
string JsonReader::_read(const string &file_path)
{
    string out = "";
    string line;
    fstream file(file_path, ios::in);
    if (!file.is_open())
    {
        throw FileNotFoundError(file_path);
    }
    while (getline(file, line))
    {
        out += line;
    }
    file.close();
    return out;
};
void JsonReader::_write(const string &file_path, const string &data)
{
    fstream file(file_path, ios::out);
    if (!file.is_open())
    {
        throw FileNotFoundError(file_path);
    }
    file << data;
    file.close();
};

JsonValue JsonReader::read(const string &file_path)
{
    string data = _read(file_path);
    StringParserPtr ptr{std::move(data)};
    try
    {
        return JsonValue::from_string(ptr);
    }
    catch (const PtrOutOfBounds &error)
    {
        throw JsonParseError();
    }
};
void JsonReader::write(const string &file_path, JsonValue &data)
{
    _write(file_path, data.to_string());
};