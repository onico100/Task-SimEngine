#include "pwse/Json.hpp"

#include <cctype>
#include <cmath>
#include <sstream>

namespace pwse::json {

double Value::asNumber() const {
    if (kind_ != Kind::Number) throw std::runtime_error("JSON value is not a number");
    return number_;
}

int Value::asInt() const { return static_cast<int>(asNumber()); }

const std::string& Value::asString() const {
    if (kind_ != Kind::String) throw std::runtime_error("JSON value is not a string");
    return string_;
}

bool Value::asBool() const {
    if (kind_ != Kind::Bool) throw std::runtime_error("JSON value is not a bool");
    return bool_;
}

const Array& Value::asArray() const {
    if (kind_ != Kind::Array) throw std::runtime_error("JSON value is not an array");
    return *array_;
}

const Object& Value::asObject() const {
    if (kind_ != Kind::Object) throw std::runtime_error("JSON value is not an object");
    return *object_;
}

const Value& Value::at(const std::string& key) const {
    const auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end()) {
        throw std::runtime_error("Missing required JSON field: '" + key + "'");
    }
    return it->second;
}

const Value* Value::find(const std::string& key) const {
    if (kind_ != Kind::Object) return nullptr;
    auto it = object_->find(key);
    return it == object_->end() ? nullptr : &it->second;
}

namespace {

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text) {}

    Value parse() {
        skipWhitespace();
        Value v = parseValue();
        skipWhitespace();
        if (pos_ != text_.size()) {
            fail("Unexpected trailing characters after JSON value");
        }
        return v;
    }

private:
    const std::string& text_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& msg) const {
        std::ostringstream oss;
        oss << "JSON parse error at offset " << pos_ << ": " << msg;
        throw std::runtime_error(oss.str());
    }

    char peek() const {
        if (pos_ >= text_.size()) fail("Unexpected end of input");
        return text_[pos_];
    }

    char advance() { return text_[pos_++]; }

    bool atEnd() const { return pos_ >= text_.size(); }

    void skipWhitespace() {
        while (!atEnd() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
            ++pos_;
        }
    }

    void expect(char c) {
        if (atEnd() || text_[pos_] != c) {
            fail(std::string("Expected '") + c + "'");
        }
        ++pos_;
    }

    Value parseValue() {
        skipWhitespace();
        if (atEnd()) fail("Unexpected end of input while parsing value");
        char c = peek();
        switch (c) {
            case '{': return parseObject();
            case '[': return parseArray();
            case '"': return Value(parseString());
            case 't': case 'f': return parseBool();
            case 'n': return parseNull();
            default:
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
                    return parseNumber();
                }
                fail(std::string("Unexpected character '") + c + "'");
        }
    }

    Value parseObject() {
        expect('{');
        Object obj;
        skipWhitespace();
        if (!atEnd() && peek() == '}') { advance(); return Value(std::move(obj)); }

        while (true) {
            skipWhitespace();
            if (peek() != '"') fail("Expected string key in object");
            std::string key = parseString();
            skipWhitespace();
            expect(':');
            skipWhitespace();
            Value val = parseValue();
            obj.emplace(std::move(key), std::move(val));
            skipWhitespace();
            if (atEnd()) fail("Unterminated object");
            char c = advance();
            if (c == ',') continue;
            if (c == '}') break;
            fail("Expected ',' or '}' in object");
        }
        return Value(std::move(obj));
    }

    Value parseArray() {
        expect('[');
        Array arr;
        skipWhitespace();
        if (!atEnd() && peek() == ']') { advance(); return Value(std::move(arr)); }

        while (true) {
            skipWhitespace();
            arr.push_back(parseValue());
            skipWhitespace();
            if (atEnd()) fail("Unterminated array");
            char c = advance();
            if (c == ',') continue;
            if (c == ']') break;
            fail("Expected ',' or ']' in array");
        }
        return Value(std::move(arr));
    }

    std::string parseString() {
        expect('"');
        std::string out;
        while (true) {
            if (atEnd()) fail("Unterminated string");
            char c = advance();
            if (c == '"') break;
            if (c == '\\') {
                if (atEnd()) fail("Unterminated escape sequence");
                char esc = advance();
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'u':
                        // Minimal support: skip 4 hex digits, emit '?' placeholder.
                        // Config files aren't expected to need full unicode escapes.
                        for (int i = 0; i < 4 && !atEnd(); ++i) advance();
                        out += '?';
                        break;
                    default: fail("Invalid escape character");
                }
            } else {
                out += c;
            }
        }
        return out;
    }

    Value parseBool() {
        if (text_.compare(pos_, 4, "true") == 0) { pos_ += 4; return Value(true); }
        if (text_.compare(pos_, 5, "false") == 0) { pos_ += 5; return Value(false); }
        fail("Invalid literal, expected 'true' or 'false'");
    }

    Value parseNull() {
        if (text_.compare(pos_, 4, "null") == 0) { pos_ += 4; return Value(); }
        fail("Invalid literal, expected 'null'");
    }

    Value parseNumber() {
        std::size_t start = pos_;
        if (!atEnd() && peek() == '-') advance();
        while (!atEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
        if (!atEnd() && peek() == '.') {
            advance();
            while (!atEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
        }
        if (!atEnd() && (peek() == 'e' || peek() == 'E')) {
            advance();
            if (!atEnd() && (peek() == '+' || peek() == '-')) advance();
            while (!atEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
        }
        std::string numStr = text_.substr(start, pos_ - start);
        try {
            return Value(std::stod(numStr));
        } catch (...) {
            fail("Invalid number literal '" + numStr + "'");
        }
    }
};

} // namespace

Value parse(const std::string& text) {
    Parser parser(text);
    return parser.parse();
}

} // namespace pwse::json
