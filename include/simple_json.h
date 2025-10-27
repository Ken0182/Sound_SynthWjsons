#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <sstream>
#include <stdexcept>

namespace simple_json {

class Reader;

class Value {
public:
    enum Type { NULL_TYPE, BOOL, NUMBER, STRING, ARRAY, OBJECT };
    
    Value() : type_(NULL_TYPE) {}
    Value(bool b) : type_(BOOL), bool_value_(b) {}
    Value(double d) : type_(NUMBER), number_value_(d) {}
    Value(const std::string& s) : type_(STRING), string_value_(s) {}
    Value(const char* s) : type_(STRING), string_value_(s) {}
    
    Type type() const { return type_; }
    
    bool asBool() const {
        if (type_ != BOOL) throw std::runtime_error("Not a boolean");
        return bool_value_;
    }
    
    double asDouble() const {
        if (type_ != NUMBER) throw std::runtime_error("Not a number");
        return number_value_;
    }
    
    std::string asString() const {
        if (type_ != STRING) throw std::runtime_error("Not a string");
        return string_value_;
    }
    
    bool isBool() const { return type_ == BOOL; }
    bool isNumber() const { return type_ == NUMBER; }
    bool isString() const { return type_ == STRING; }
    bool isArray() const { return type_ == ARRAY; }
    bool isObject() const { return type_ == OBJECT; }
    
    // Object access
    bool isMember(const std::string& key) const {
        if (type_ != OBJECT) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    const Value& operator[](const std::string& key) const {
        if (type_ != OBJECT) throw std::runtime_error("Not an object");
        auto it = object_value_.find(key);
        if (it == object_value_.end()) throw std::runtime_error("Key not found");
        return it->second;
    }
    
    Value& operator[](const std::string& key) {
        if (type_ != OBJECT) throw std::runtime_error("Not an object");
        return object_value_[key];
    }
    
    const Value& operator[](size_t index) const {
        if (type_ != ARRAY) throw std::runtime_error("Not an array");
        if (index >= array_value_.size()) throw std::runtime_error("Index out of range");
        return array_value_[index];
    }
    
    Value& operator[](size_t index) {
        if (type_ != ARRAY) throw std::runtime_error("Not an array");
        if (index >= array_value_.size()) throw std::runtime_error("Index out of range");
        return array_value_[index];
    }
    
    size_t size() const {
        if (type_ == ARRAY) return array_value_.size();
        if (type_ == OBJECT) return object_value_.size();
        return 0;
    }
    
    std::vector<std::string> getMemberNames() const {
        if (type_ != OBJECT) return {};
        std::vector<std::string> names;
        for (const auto& pair : object_value_) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    void append(const Value& value) {
        if (type_ != ARRAY) throw std::runtime_error("Not an array");
        array_value_.push_back(value);
    }
    
    // Friend class to access private members
    friend class Reader;
    
private:
    Type type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    std::vector<Value> array_value_;
    std::map<std::string, Value> object_value_;
};

class Reader {
public:
    bool parse(const std::string& json, Value& root) {
        try {
            root = parseValue(json, 0).first;
            return true;
        } catch (const std::exception& e) {
            error_message_ = e.what();
            return false;
        }
    }
    
    std::string getFormattedErrorMessages() const {
        return error_message_;
    }
    
private:
    std::string error_message_;
    
    std::pair<Value, size_t> parseValue(const std::string& json, size_t pos) {
        skipWhitespace(json, pos);
        
        if (pos >= json.length()) {
            throw std::runtime_error("Unexpected end of input");
        }
        
        char c = json[pos];
        
        if (c == '{') {
            return parseObject(json, pos);
        } else if (c == '[') {
            return parseArray(json, pos);
        } else if (c == '"') {
            return parseString(json, pos);
        } else if (c == 't' || c == 'f') {
            return parseBool(json, pos);
        } else if (c == 'n') {
            return parseNull(json, pos);
        } else if (c == '-' || (c >= '0' && c <= '9')) {
            return parseNumber(json, pos);
        } else {
            throw std::runtime_error("Unexpected character: " + std::string(1, c));
        }
    }
    
    std::pair<Value, size_t> parseObject(const std::string& json, size_t pos) {
        Value obj;
        obj.type_ = Value::OBJECT;
        
        pos++; // skip '{'
        skipWhitespace(json, pos);
        
        if (pos < json.length() && json[pos] == '}') {
            return {obj, pos + 1};
        }
        
        while (pos < json.length()) {
            skipWhitespace(json, pos);
            
            // Parse key
            auto [key, keyEnd] = parseString(json, pos);
            std::string keyStr = key.asString();
            
            pos = keyEnd;
            skipWhitespace(json, pos);
            
            if (pos >= json.length() || json[pos] != ':') {
                throw std::runtime_error("Expected ':' after key");
            }
            pos++; // skip ':'
            
            // Parse value
            auto [value, valueEnd] = parseValue(json, pos);
            obj.object_value_[keyStr] = value;
            
            pos = valueEnd;
            skipWhitespace(json, pos);
            
            if (pos >= json.length()) {
                throw std::runtime_error("Unexpected end of input in object");
            }
            
            if (json[pos] == '}') {
                return {obj, pos + 1};
            } else if (json[pos] == ',') {
                pos++; // skip ','
            } else {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
        }
        
        throw std::runtime_error("Unexpected end of input in object");
    }
    
    std::pair<Value, size_t> parseArray(const std::string& json, size_t pos) {
        Value arr;
        arr.type_ = Value::ARRAY;
        
        pos++; // skip '['
        skipWhitespace(json, pos);
        
        if (pos < json.length() && json[pos] == ']') {
            return {arr, pos + 1};
        }
        
        while (pos < json.length()) {
            auto [value, valueEnd] = parseValue(json, pos);
            arr.array_value_.push_back(value);
            
            pos = valueEnd;
            skipWhitespace(json, pos);
            
            if (pos >= json.length()) {
                throw std::runtime_error("Unexpected end of input in array");
            }
            
            if (json[pos] == ']') {
                return {arr, pos + 1};
            } else if (json[pos] == ',') {
                pos++; // skip ','
            } else {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
        }
        
        throw std::runtime_error("Unexpected end of input in array");
    }
    
    std::pair<Value, size_t> parseString(const std::string& json, size_t pos) {
        if (pos >= json.length() || json[pos] != '"') {
            throw std::runtime_error("Expected '\"' at start of string");
        }
        
        pos++; // skip opening quote
        std::string result;
        
        while (pos < json.length() && json[pos] != '"') {
            if (json[pos] == '\\') {
                pos++;
                if (pos >= json.length()) {
                    throw std::runtime_error("Unexpected end of input in string");
                }
                
                switch (json[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += json[pos]; break;
                }
            } else {
                result += json[pos];
            }
            pos++;
        }
        
        if (pos >= json.length() || json[pos] != '"') {
            throw std::runtime_error("Unterminated string");
        }
        
        return {Value(result), pos + 1};
    }
    
    std::pair<Value, size_t> parseBool(const std::string& json, size_t pos) {
        if (pos + 4 <= json.length() && json.substr(pos, 4) == "true") {
            return {Value(true), pos + 4};
        } else if (pos + 5 <= json.length() && json.substr(pos, 5) == "false") {
            return {Value(false), pos + 5};
        } else {
            throw std::runtime_error("Invalid boolean value");
        }
    }
    
    std::pair<Value, size_t> parseNull(const std::string& json, size_t pos) {
        if (pos + 4 <= json.length() && json.substr(pos, 4) == "null") {
            return {Value(), pos + 4};
        } else {
            throw std::runtime_error("Invalid null value");
        }
    }
    
    std::pair<Value, size_t> parseNumber(const std::string& json, size_t pos) {
        size_t start = pos;
        
        if (json[pos] == '-') pos++;
        
        if (pos >= json.length() || !std::isdigit(json[pos])) {
            throw std::runtime_error("Invalid number");
        }
        
        while (pos < json.length() && std::isdigit(json[pos])) {
            pos++;
        }
        
        if (pos < json.length() && json[pos] == '.') {
            pos++;
            if (pos >= json.length() || !std::isdigit(json[pos])) {
                throw std::runtime_error("Invalid number");
            }
            while (pos < json.length() && std::isdigit(json[pos])) {
                pos++;
            }
        }
        
        if (pos < json.length() && (json[pos] == 'e' || json[pos] == 'E')) {
            pos++;
            if (pos < json.length() && (json[pos] == '+' || json[pos] == '-')) {
                pos++;
            }
            if (pos >= json.length() || !std::isdigit(json[pos])) {
                throw std::runtime_error("Invalid number");
            }
            while (pos < json.length() && std::isdigit(json[pos])) {
                pos++;
            }
        }
        
        std::string numStr = json.substr(start, pos - start);
        double value = std::stod(numStr);
        
        return {Value(value), pos};
    }
    
    void skipWhitespace(const std::string& json, size_t& pos) {
        while (pos < json.length() && std::isspace(json[pos])) {
            pos++;
        }
    }
};

} // namespace simple_json
