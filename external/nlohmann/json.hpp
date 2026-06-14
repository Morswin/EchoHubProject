// This is a minimal stub for nlohmann/json to allow compilation
// In a real project, you should use the full library from:
// https://github.com/nlohmann/json

#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nlohmann {
    // Forward declarations
    template<typename T>
    class basic_json;

    using json = basic_json<std::map>;

    // Simplified basic_json implementation for our needs
    template<typename ObjectType>
    class basic_json {
    public:
        // Types
        enum class value_t { null, object, array, string, boolean, number, discarded };

        // Constructors
        basic_json() noexcept : m_type(value_t::null) {}
        basic_json(std::nullptr_t) noexcept : m_type(value_t::null) {}
        basic_json(bool b) : m_type(value_t::boolean), m_value(b) {}
        basic_json(int i) : m_type(value_t::number), m_value(static_cast<double>(i)) {}
        basic_json(double d) : m_type(value_t::number), m_value(d) {}
        basic_json(const std::string& s) : m_type(value_t::string), m_value(s) {}
        basic_json(const char* s) : m_type(value_t::string), m_value(std::string(s)) {}
        basic_json(std::initializer_list<std::pair<const std::string, basic_json>> init) : m_type(value_t::object) {
            for (const auto& p : init) {
                m_value.emplace_back(p.first, p.second);
            }
        }

        // Accessors
        value_t type() const noexcept { return m_type; }
        bool is_null() const noexcept { return m_type == value_t::null; }
        bool is_boolean() const noexcept { return m_type == value_t::boolean; }
        bool is_number() const noexcept { return m_type == value_t::number; }
        bool is_string() const noexcept { return m_type == value_t::string; }
        bool is_object() const noexcept { return m_type == value_t::object; }
        bool is_array() const noexcept { return m_type == value_t::array; }

        // Get values
        bool get<bool>() const { return std::get<bool>(m_value); }
        double get<double>() const { return std::get<double>(m_value); }
        std::string get<std::string>() const { return std::get<std::string>(m_value); }
        ObjectType get<ObjectType>() const { return std::get<ObjectType>(m_value); }

        // Array access
        basic_json& operator[](size_t idx) {
            if (m_type != value_t::array) throw std::runtime_error("not an array");
            auto& arr = std::get<std::vector<basic_json>>(m_value);
            if (idx >= arr.size()) arr.resize(idx + 1);
            return arr[idx];
        }

        // Object access
        basic_json& operator[](const std::string& key) {
            if (m_type != value_t::object) {
                m_type = value_t::object;
                m_value = ObjectType{};
            }
            auto& obj = std::get<ObjectType>(m_value);
            return obj[key];
        }

        const basic_json& operator[](const std::string& key) const {
            if (m_type != value_t::object) throw std::runtime_error("not an object");
            const auto& obj = std::get<ObjectType>(m_value);
            auto it = obj.find(key);
            if (it == obj.end()) throw std::runtime_error("key not found");
            return it->second;
        }

        // Check if key exists
        bool contains(const std::string& key) const {
            if (m_type != value_t::object) return false;
            const auto& obj = std::get<ObjectType>(m_value);
            return obj.find(key) != obj.end();
        }

        // Get value with default
        template<typename T>
        T value(const std::string& key, T default_value) const {
            if (m_type != value_t::object) return default_value;
            const auto& obj = std::get<ObjectType>(m_value);
            auto it = obj.find(key);
            if (it == obj.end()) return default_value;
            return it->second.get<T>();
        }

        template<typename T>
        T value(const std::string& key) const {
            if (m_type != value_t::object) throw std::runtime_error("not an object");
            const auto& obj = std::get<ObjectType>(m_value);
            auto it = obj.find(key);
            if (it == obj.end()) throw std::runtime_error("key not found");
            return it->second.get<T>();
        }

        // Array methods
        void push_back(const basic_json& j) {
            if (m_type != value_t::array) {
                m_type = value_t::array;
                m_value = std::vector<basic_json>{};
            }
            auto& arr = std::get<std::vector<basic_json>>(m_value);
            arr.push_back(j);
        }

        size_t size() const {
            if (m_type == value_t::array) {
                return std::get<std::vector<basic_json>>(m_value).size();
            } else if (m_type == value_t::object) {
                return std::get<ObjectType>(m_value).size();
            }
            return 0;
        }

        // Serialization
        std::string dump(int indent = -1) const {
            std::ostringstream oss;
            dump(oss, indent, 0);
            return oss.str();
        }

        // Parse from string
        static basic_json parse(const std::string& str) {
            basic_json j;
            size_t pos = 0;
            j.parse_internal(str, pos);
            return j;
        }

    private:
        value_t m_type;
        std::variant<std::monostate, bool, double, std::string, ObjectType, std::vector<basic_json>> m_value;

        void dump(std::ostream& oss, int indent, int current_indent) const {
            switch (m_type) {
                case value_t::null:
                    oss << "null";
                    break;
                case value_t::boolean:
                    oss << (std::get<bool>(m_value) ? "true" : "false");
                    break;
                case value_t::number:
                    oss << std::get<double>(m_value);
                    break;
                case value_t::string: {
                    const auto& s = std::get<std::string>(m_value);
                    oss << '"' << escape_string(s) << '"';
                    break;
                }
                case value_t::object: {
                    const auto& obj = std::get<ObjectType>(m_value);
                    oss << "{";
                    bool first = true;
                    for (const auto& p : obj) {
                        if (!first) oss << ",";
                        if (indent >= 0) oss << "\n" << std::string(current_indent + indent, ' ');
                        oss << '"' << escape_string(p.first) << '":';
                        if (indent >= 0) oss << " ";
                        p.second.dump(oss, indent, current_indent + indent);
                        first = false;
                    }
                    if (indent >= 0 && !obj.empty()) oss << "\n" << std::string(current_indent, ' ');
                    oss << "}";
                    break;
                }
                case value_t::array: {
                    const auto& arr = std::get<std::vector<basic_json>>(m_value);
                    oss << "[";
                    bool first = true;
                    for (const auto& j : arr) {
                        if (!first) oss << ",";
                        if (indent >= 0) oss << "\n" << std::string(current_indent + indent, ' ');
                        j.dump(oss, indent, current_indent + indent);
                        first = false;
                    }
                    if (indent >= 0 && !arr.empty()) oss << "\n" << std::string(current_indent, ' ');
                    oss << "]";
                    break;
                }
                case value_t::discarded:
                    break;
            }
        }

        void parse_internal(const std::string& str, size_t& pos) {
            skip_whitespace(str, pos);
            if (pos >= str.size()) return;

            char c = str[pos];
            if (c == '{') {
                parse_object(str, pos);
            } else if (c == '[') {
                parse_array(str, pos);
            } else if (c == '"') {
                parse_string(str, pos);
            } else if (c == 't' && str.substr(pos, 4) == "true") {
                m_type = value_t::boolean;
                m_value = true;
                pos += 4;
            } else if (c == 'f' && str.substr(pos, 5) == "false") {
                m_type = value_t::boolean;
                m_value = false;
                pos += 5;
            } else if (c == 'n' && str.substr(pos, 4) == "null") {
                m_type = value_t::null;
                pos += 4;
            } else if (c == '-' || isdigit(c)) {
                parse_number(str, pos);
            }
        }

        void parse_object(const std::string& str, size_t& pos) {
            m_type = value_t::object;
            m_value = ObjectType{};
            pos++; // skip '{'
            skip_whitespace(str, pos);

            while (pos < str.size() && str[pos] != '}') {
                skip_whitespace(str, pos);
                if (str[pos] == '"') {
                    std::string key = parse_string_value(str, pos);
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == ':') {
                        pos++;
                        skip_whitespace(str, pos);
                        basic_json value;
                        value.parse_internal(str, pos);
                        std::get<ObjectType>(m_value)[key] = value;
                    }
                }
                skip_whitespace(str, pos);
                if (pos < str.size() && str[pos] == ',') {
                    pos++;
                }
            }
            if (pos < str.size() && str[pos] == '}') pos++;
        }

        void parse_array(const std::string& str, size_t& pos) {
            m_type = value_t::array;
            m_value = std::vector<basic_json>{};
            pos++; // skip '['
            skip_whitespace(str, pos);

            while (pos < str.size() && str[pos] != ']') {
                skip_whitespace(str, pos);
                basic_json value;
                value.parse_internal(str, pos);
                std::get<std::vector<basic_json>>(m_value).push_back(value);
                skip_whitespace(str, pos);
                if (pos < str.size() && str[pos] == ',') {
                    pos++;
                }
            }
            if (pos < str.size() && str[pos] == ']') pos++;
        }

        void parse_string(const std::string& str, size_t& pos) {
            m_type = value_t::string;
            m_value = parse_string_value(str, pos);
        }

        void parse_number(const std::string& str, size_t& pos) {
            m_type = value_t::number;
            size_t start = pos;
            if (str[pos] == '-') pos++;
            while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.' || str[pos] == 'e' || str[pos] == 'E' || str[pos] == '+' || str[pos] == '-')) {
                pos++;
            }
            std::string num_str = str.substr(start, pos - start);
            try {
                m_value = std::stod(num_str);
            } catch (...) {
                m_value = 0.0;
            }
        }

        static std::string parse_string_value(const std::string& str, size_t& pos) {
            pos++; // skip '"'
            std::string result;
            bool escape = false;
            while (pos < str.size() && (escape || str[pos] != '"')) {
                if (escape) {
                    switch (str[pos]) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        default: result += str[pos]; break;
                    }
                    escape = false;
                } else if (str[pos] == '\\') {
                    escape = true;
                } else {
                    result += str[pos];
                }
                pos++;
            }
            if (pos < str.size() && str[pos] == '"') pos++;
            return result;
        }

        static void skip_whitespace(const std::string& str, size_t& pos) {
            while (pos < str.size() && isspace(str[pos])) pos++;
        }

        static std::string escape_string(const std::string& s) {
            std::string result;
            for (char c : s) {
                switch (c) {
                    case '"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\b': result += "\\b"; break;
                    case '\f': result += "\\f"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: result += c; break;
                }
            }
            return result;
        }
    };

    // Alias for convenience
    using json = basic_json<std::map>;
}

#endif // NLOHMANN_JSON_HPP
