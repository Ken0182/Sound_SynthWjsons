// This is a placeholder for nlohmann/json single header
// In a real implementation, you would download the actual nlohmann/json.hpp
// from https://github.com/nlohmann/json/releases

#pragma once

// For now, we'll create a minimal JSON implementation that's compatible
// with our needs and doesn't have string_view issues

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <stdexcept>

namespace nlohmann {

class json {
public:
    using value_t = std::variant<std::nullptr_t, bool, int64_t, double, std::string, std::vector<json>, std::map<std::string, json>>;
    
    value_t value;
    
    json() : value(nullptr) {}
    json(std::nullptr_t) : value(nullptr) {}
    json(bool b) : value(b) {}
    json(int64_t i) : value(i) {}
    json(double d) : value(d) {}
    json(const std::string& s) : value(s) {}
    json(const char* s) : value(std::string(s)) {}
    json(const std::vector<json>& v) : value(v) {}
    json(const std::map<std::string, json>& m) : value(m) {}
    
    // Type checking
    bool is_null() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool is_boolean() const { return std::holds_alternative<bool>(value); }
    bool is_number_integer() const { return std::holds_alternative<int64_t>(value); }
    bool is_number_float() const { return std::holds_alternative<double>(value); }
    bool is_string() const { return std::holds_alternative<std::string>(value); }
    bool is_array() const { return std::holds_alternative<std::vector<json>>(value); }
    bool is_object() const { return std::holds_alternative<std::map<std::string, json>>(value); }
    
    // Value access
    bool get_bool() const { return std::get<bool>(value); }
    int64_t get_int() const { return std::get<int64_t>(value); }
    double get_double() const { return std::get<double>(value); }
    const std::string& get_string() const { return std::get<std::string>(value); }
    const std::vector<json>& get_array() const { return std::get<std::vector<json>>(value); }
    const std::map<std::string, json>& get_object() const { return std::get<std::map<std::string, json>>(value); }
    
    // Array access
    json& operator[](size_t index) {
        if (!is_array()) {
            value = std::vector<json>();
        }
        auto& arr = std::get<std::vector<json>>(value);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }
    
    const json& operator[](size_t index) const {
        if (!is_array()) {
            throw std::runtime_error("Not an array");
        }
        const auto& arr = std::get<std::vector<json>>(value);
        if (index >= arr.size()) {
            throw std::runtime_error("Index out of range");
        }
        return arr[index];
    }
    
    // Object access
    json& operator[](const std::string& key) {
        if (!is_object()) {
            value = std::map<std::string, json>();
        }
        return std::get<std::map<std::string, json>>(value)[key];
    }
    
    const json& operator[](const std::string& key) const {
        if (!is_object()) {
            throw std::runtime_error("Not an object");
        }
        const auto& obj = std::get<std::map<std::string, json>>(value);
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::runtime_error("Key not found: " + key);
        }
        return it->second;
    }
    
    // Object methods
    bool contains(const std::string& key) const {
        if (!is_object()) return false;
        const auto& obj = std::get<std::map<std::string, json>>(value);
        return obj.find(key) != obj.end();
    }
    
    size_t size() const {
        if (is_array()) return std::get<std::vector<json>>(value).size();
        if (is_object()) return std::get<std::map<std::string, json>>(value).size();
        return 0;
    }
    
    // Iterator support for objects
    auto begin() -> decltype(std::get<std::map<std::string, json>>(value).begin()) {
        if (!is_object()) {
            value = std::map<std::string, json>();
        }
        return std::get<std::map<std::string, json>>(value).begin();
    }
    
    auto end() -> decltype(std::get<std::map<std::string, json>>(value).end()) {
        if (!is_object()) {
            value = std::map<std::string, json>();
        }
        return std::get<std::map<std::string, json>>(value).end();
    }
    
    auto begin() const -> decltype(std::get<std::map<std::string, json>>(value).begin()) {
        if (!is_object()) {
            throw std::runtime_error("Not an object");
        }
        return std::get<std::map<std::string, json>>(value).begin();
    }
    
    auto end() const -> decltype(std::get<std::map<std::string, json>>(value).end()) {
        if (!is_object()) {
            throw std::runtime_error("Not an object");
        }
        return std::get<std::map<std::string, json>>(value).end();
    }
};

// Parser (simplified)
class json_parser {
public:
    static json parse(const std::string& json_string) {
        // This is a simplified parser - in a real implementation,
        // you would use a proper JSON parsing library
        // For now, return a basic object
        return json(std::map<std::string, json>());
    }
    
    static std::string dump(const json& j, int indent = -1) {
        // Simplified dumper - in a real implementation,
        // you would use a proper JSON serialization library
        return "{}";
    }
};

} // namespace nlohmann