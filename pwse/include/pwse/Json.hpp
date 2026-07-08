#pragma once

// A minimal, dependency-free JSON parser sufficient for reading simulation
// config files. Not a general-purpose JSON library: no comments, no
// streaming, modest error messages. Good enough for MVP config loading;
// swap for a real library (e.g. nlohmann/json) later if requirements grow.

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace pwse::json {

class Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value>;

enum class Kind { Null, Bool, Number, String, Array, Object };

class Value {
public:
    Value() : kind_(Kind::Null) {}
    Value(bool b) : kind_(Kind::Bool), bool_(b) {}
    Value(double n) : kind_(Kind::Number), number_(n) {}
    Value(std::string s) : kind_(Kind::String), string_(std::move(s)) {}
    Value(Array a) : kind_(Kind::Array), array_(std::make_shared<Array>(std::move(a))) {}
    Value(Object o) : kind_(Kind::Object), object_(std::make_shared<Object>(std::move(o))) {}

    Kind kind() const { return kind_; }
    bool isNull() const { return kind_ == Kind::Null; }
    bool isObject() const { return kind_ == Kind::Object; }
    bool isArray() const { return kind_ == Kind::Array; }

    double asNumber() const;
    int asInt() const;
    const std::string& asString() const;
    bool asBool() const;
    const Array& asArray() const;
    const Object& asObject() const;

    // Object field access with a required/optional variant.
    const Value& at(const std::string& key) const;      // throws if missing
    const Value* find(const std::string& key) const;     // nullptr if missing

private:
    Kind kind_;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    std::shared_ptr<Array> array_;
    std::shared_ptr<Object> object_;
};

// Parses `text` as JSON. Throws std::runtime_error with a descriptive
// message (including position) on malformed input.
Value parse(const std::string& text);

} // namespace pwse::json
