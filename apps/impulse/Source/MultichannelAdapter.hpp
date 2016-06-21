#pragma once

#include <memory>
#include <vector>

template <typename T>
class MultichannelAdapter {
public:
    using input_type = typename T::input_type;
    using output_type = typename T::output_type;

    template <typename... Ts>
    MultichannelAdapter(size_t num_channels, Ts&&... ts) {
        for (auto i = 0u; i != num_channels; ++i) {
            channels.emplace_back(std::make_unique<T>(std::forward<Ts>(ts)...));
        }
    }

    bool has_waiting_frames() const {
        return channels.front()->has_waiting_frames();
    }

    size_t get_num_channels() const {
        return channels.size();
    }

    size_t get_window_size() const {
        return channels.front()->get_window_size();
    }

    size_t get_hop_size() const {
        return channels.front()->get_hop_size();
    }

    void write(const input_type** begin, size_t num) {
        auto lim = get_num_channels();
        for (auto i = 0u; i != lim; ++i) {
            channels[i]->write(begin[i], num);
        }
    }

    std::vector<output_type> read_frame() {
        std::vector<output_type> ret(get_num_channels());
        std::transform(channels.begin(),
                       channels.end(),
                       ret.begin(),
                       [](const auto& i) { return i->read_frame(); });
        return ret;
    }

private:
    std::vector<std::unique_ptr<T>> channels;
};
