#pragma once

#include <exception>

namespace core {
namespace exceptions {

class exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

protected:
    ~exception() noexcept = default;
};

class suspicious_value : public exception {
    using exception::exception;

protected:
    ~suspicious_value() noexcept = default;
};

class value_is_nan final : public suspicious_value {
    using suspicious_value::suspicious_value;
};

class value_is_inf final : public suspicious_value {
    using suspicious_value::suspicious_value;
};

}  // namespace exceptions
}  // namespace core
