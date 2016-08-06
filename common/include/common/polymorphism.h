#pragma once

template <typename base>
class abstract_cloneable {
public:
    abstract_cloneable() = default;
    abstract_cloneable(const abstract_cloneable&) noexcept = default;
    abstract_cloneable& operator=(const abstract_cloneable&) noexcept = default;
    abstract_cloneable(abstract_cloneable&&) noexcept = default;
    abstract_cloneable& operator=(abstract_cloneable&&) noexcept = default;
    virtual ~abstract_cloneable() noexcept = default;

    virtual std::unique_ptr<base> clone() const = 0;
};

///	To use:
///		class my_derived : public cloneable_base<my_derived, the_base_class>;
template <typename derived, typename base>
class cloneable_base : public base {
public:
    std::unique_ptr<base> clone() const override {
        return std::make_unique<derived>(*(static_cast<const derived*>(this)));
    }
};
