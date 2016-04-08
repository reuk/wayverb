#pragma once

#include "vec_forward.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/rapidjson.h"

#include "cl_include.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/// This file contains a set of classes which serve to form a layer over the
/// rapidjson parser, which can easily be used to validate and read json
/// fields into specific types.
/// To validate a specific type, just add a specialization of JsonGetter<>
/// for the type that you want to validate.
///
/// If there is a god this will be deprecated soon.

namespace config {

void attempt_json_parse(const std::string& fname, rapidjson::Document& doc);

/// A simple interface for a JsonValidator.
struct JsonValidatorBase {
    virtual ~JsonValidatorBase() noexcept = default;
    /// Overload to dictate what happens when a JsonValidator is Run on a Value
    virtual void run(const rapidjson::Value& value) const = 0;
};

struct OptionalValidator;
struct RequiredValidator;

template <typename T>
struct ValueJsonValidator;
template <typename T, typename U>
struct FieldJsonValidator;

/// Used to register required and optional fields that should be present in a
/// config file.
/// Also has the ability to parse a value for these required and optional
/// fields.
/// You almost definitely want an instance of THIS CLASS rather than any other.
class ConfigValidator : public JsonValidatorBase {
public:
    template <typename T>
    void addOptionalValidator(const std::string& s, T& t) {
        validators.emplace_back(
            std::make_unique<FieldJsonValidator<T, OptionalValidator>>(s, t));
    }

    template <typename T>
    void addRequiredValidator(const std::string& s, T& t) {
        validators.emplace_back(
            std::make_unique<FieldJsonValidator<T, RequiredValidator>>(s, t));
    }

    void run(const rapidjson::Value& value) const override {
        for (const auto& i : validators) {
            i->run(value);
        }
    }

private:
    std::vector<std::unique_ptr<JsonValidatorBase>> validators;
};

/// This is basically just an immutable string.
struct StringWrapper {
    explicit StringWrapper(const std::string& s)
            : s(s) {
    }
    virtual ~StringWrapper() noexcept = default;
    const std::string& getString() const {
        return s;
    }

private:
    const std::string& s;
};

/// An interface for a json value validator.
/// Set it up with a string, and then supply some way of validating whether
/// the value is valid for the given string.
struct Validator : public StringWrapper {
    explicit Validator(const std::string& s)
            : StringWrapper(s) {
    }
    virtual bool validate(const rapidjson::Value& value) const = 0;
};

/// Specialised Validator.
/// If the value does not contain a member matching the validator's string, an
/// exception is thrown.
/// Otherwise, validate returns true.
struct RequiredValidator : public Validator {
    explicit RequiredValidator(const std::string& s)
            : Validator(s) {
    }
    bool validate(const rapidjson::Value& value) const override {
        if (!value.HasMember(getString().c_str())) {
            throw std::runtime_error("key " + getString() +
                                     " not found in config object");
        }
        return true;
    }
};

/// Specialised Validator.
/// Returns true if the value contains a member matching the validator's string,
/// false otherwise.
struct OptionalValidator : public Validator {
    explicit OptionalValidator(const std::string& s)
            : Validator(s) {
    }
    bool validate(const rapidjson::Value& value) const override {
        return value.HasMember(getString().c_str());
    }
};

/// A class that can validate and mutate a stored, templated value based on a
/// supplied json value.
template <typename T>
struct JsonGetter {
    virtual ~JsonGetter() noexcept = default;
    virtual bool check(const rapidjson::Value& value) const = 0;
    virtual void get(const rapidjson::Value& value) const = 0;
};

template <typename T>
auto get_json_getter(T& t) {
    return JsonGetter<T>(t);
}

template <>
struct JsonGetter<double> {
    explicit JsonGetter(double& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a number, false otherwise.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsNumber();
    }

    /// Gets the value as a double.
    virtual void get(const rapidjson::Value& value) const {
        t = value.GetDouble();
    }
    double& t;
};

template <>
struct JsonGetter<float> {
    explicit JsonGetter(float& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is a number, false otherwise.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsNumber();
    }

    /// Gets the value as a double, then casts it to float.
    virtual void get(const rapidjson::Value& value) const {
        t = value.GetDouble();
    }
    float& t;
};

template <>
struct JsonGetter<bool> {
    explicit JsonGetter(bool& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is 'true' or 'false', false otherwise.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsBool();
    }

    /// Converts json bool to C++ bool.
    virtual void get(const rapidjson::Value& value) const {
        t = value.GetBool();
    }
    bool& t;
};

template <>
struct JsonGetter<int> {
    explicit JsonGetter(int& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    /// Returns true if value is an integer, false otherwise.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsInt();
    }

    /// Gets json value as integer.
    virtual void get(const rapidjson::Value& value) const {
        t = value.GetInt();
    }
    int& t;
};

/// General class for getting numerical json arrays into cl_floatx types
template <typename T, int LENGTH>
struct JsonArrayGetter {
    explicit JsonArrayGetter(T& t)
            : t(t) {
    }
    virtual ~JsonArrayGetter() noexcept = default;

    /// Return true if value is an array of length LENGTH containing only
    /// numbers.
    virtual bool check(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            return false;
        }
        if (value.Size() != LENGTH) {
            return false;
        }
        for (auto i = 0; i != LENGTH; ++i) {
            if (!value[i].IsNumber()) {
                return false;
            }
        }
        return true;
    }

    /// Gets json value as a cl_floatx.
    virtual void get(const rapidjson::Value& value) const {
        for (auto i = 0; i != LENGTH; ++i) {
            t.s[i] = static_cast<cl_float>(value[i].GetDouble());
        }
    }

    T& t;
};

/// JsonGetter for cl_float3 is just a JsonArrayGetter with length 3
template <>
struct JsonGetter<cl_float3> : public JsonArrayGetter<cl_float3, 3> {
    explicit JsonGetter(cl_float3& t)
            : JsonArrayGetter(t) {
    }
};

/// JsonGetter for cl_float8 is just a JsonArrayGetter with length 8
template <>
struct JsonGetter<cl_float8> : public JsonArrayGetter<cl_float8, 8> {
    explicit JsonGetter(cl_float8& t)
            : JsonArrayGetter(t) {
    }
};

/// General class for getting a json field with strings mapped to enums
template <typename T>
struct JsonEnumGetter {
    JsonEnumGetter(T& t, const std::map<std::string, T>& m)
            : t(t)
            , stringkeys(m) {
    }
    virtual ~JsonEnumGetter() noexcept = default;

    /// Returns true if value is a string and is equal to an allowed string.
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsString() &&
               (any_of(stringkeys.begin(),
                       stringkeys.end(),
                       [&value](const std::pair<std::string, T>& i) {
                           return i.first == value.GetString();
                       }));
    }

    /// Look up the string in a dictionary and return the equivalent enum value.
    virtual void get(const rapidjson::Value& value) const {
        t = stringkeys.at(value.GetString());
    }

    T& t;
    const std::map<std::string, T> stringkeys;
};

template <typename T>
struct JsonGetter<std::vector<T>> {
    explicit JsonGetter(std::vector<T>& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;
    virtual bool check(const rapidjson::Value& value) const {
        return value.IsArray();
    }
    virtual void get(const rapidjson::Value& value) const {
        for (auto i = value.Begin(); i != value.End(); ++i) {
            T temp;
            ValueJsonValidator<T> getter(temp);
            getter.run(*i);
            t.push_back(temp);
        }
    }
    std::vector<T>& t;
};

template <typename T>
struct ValueJsonValidator : public JsonValidatorBase, public JsonGetter<T> {
    explicit ValueJsonValidator(T& t)
            : JsonGetter<T>(t) {
    }

    void run(const rapidjson::Value& value) const override {
        if (!JsonGetter<T>::check(value)) {
            throw std::runtime_error("invalid value");
        }
        JsonGetter<T>::get(value);
    }
};

template <>
struct JsonGetter<Vec3f> {
    explicit JsonGetter(Vec3f& t)
            : t(t) {
    }
    virtual ~JsonGetter() noexcept = default;

    virtual bool check(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            return false;
        }
        if (value.Size() != 3) {
            return false;
        }
        for (auto i = 0u; i != 3; ++i) {
            if (!value[i].IsNumber()) {
                return false;
            }
        }
        return true;
    }

    virtual void get(const rapidjson::Value& value) const;

    Vec3f& t;
};

/// Combines the functionality of any templated JsonGetter with any Validator,
/// giving an object that, when run, validates that the value is present, checks
/// it, and gets it if possible.
/// It's possible to refer to instances in a typesafe way through pointers to
/// JsonValidatorBase, which allows for runtime-polymorphic JsonValidators to
/// be instantiated, depending on runtime requirements.
template <typename T, typename U>
struct FieldJsonValidator : public ValueJsonValidator<T>, public U {
    FieldJsonValidator(const std::string& s, T& t)
            : ValueJsonValidator<T>(t)
            , U(s) {
    }

    void run(const rapidjson::Value& value) const override {
        if (U::validate(value)) {
            ValueJsonValidator<T>::run(value[U::getString().c_str()]);
        }
    }
};
}  // namespace config
