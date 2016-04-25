#pragma once

#include <cereal/archives/json.hpp>
#include <fstream>

namespace json_read_write {

namespace detail {

enum class IO {
    input,
    output,
};

template <IO I>
struct IOTrait;

template <>
struct IOTrait<IO::input> {
    using fstream_type = std::ifstream;
    using archive_type = cereal::JSONInputArchive;
};

template <>
struct IOTrait<IO::output> {
    using fstream_type = std::ofstream;
    using archive_type = cereal::JSONOutputArchive;
};

template <IO io, typename... Ts>
void serialize(const std::string& path, Ts&&... ts) {
    typename IOTrait<io>::fstream_type file(path);
    typename IOTrait<io>::archive_type archive(file);
    archive(std::forward<Ts>(ts)...);
}
}  // namespace detail

template <typename... Ts>
void read(const std::string& path, Ts&&... ts) {
    detail::serialize<detail::IO::input>(path, std::forward<Ts>(ts)...);
}

template <typename... Ts>
void write(const std::string& path, Ts&&... ts) {
    detail::serialize<detail::IO::output>(path, std::forward<Ts>(ts)...);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, T&& t) {
    cereal::JSONOutputArchive archive(os);
    archive(cereal::make_nvp("serialized", std::forward<T>(t)));
    return os;
}

#define JSON_OSTREAM_OVERLOAD(type)                                    \
    inline std::ostream& operator<<(std::ostream& os, const type& t) { \
        return json_read_write::operator<<(os, t);                     \
    }

}  // namespace json_read_write
