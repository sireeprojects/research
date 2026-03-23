#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <sstream>
#include <type_traits>
#include <stdexcept>
#include <iomanip>
#include <algorithm>

namespace vtb {

class CmdlineParser {
public:
    struct Option {
        std::string short_name;
        std::string long_name;
        std::string description;
        bool expects_value = false;
        bool is_required = false;
        std::vector<std::string> allowed_values;
    };

    void add_argument(std::string_view short_n, std::string_view long_n,
                      std::string_view desc, bool has_val = false,
                      bool required = false, std::vector<std::string> allowed = {});

    void parse(int argc, char* argv[]);
    bool has(std::string_view name) const;
    void print_help() const;

    /**
     * @brief Get a single value converted to type T.
     * Handles string, bool, and numeric (including Hex 0x).
     */
    template <typename T>
    T get(std::string_view long_name) const {
        auto it = results.find(std::string(long_name));
        if (it == results.end() || it->second.empty()) return T{};
        return convert<T>(it->second.front(), long_name);
    }

    /**
     * @brief Get all values provided for a specific flag as a vector of T.
     */
    template <typename T>
    std::vector<T> get_list(std::string_view long_name) const {
        std::vector<T> output;
        auto it = results.find(std::string(long_name));
        if (it != results.end()) {
            for (const auto& s : it->second) {
                output.push_back(convert<T>(s, long_name));
            }
        }
        return output;
    }

private:
    void validate_requirements();

    template <typename T>
    T convert(const std::string& val, std::string_view name) const {
        if constexpr (std::is_same_v<T, std::string>) return val;
        if constexpr (std::is_same_v<T, bool>) return (val == "true");

        try {
            if constexpr (std::is_integral_v<T>) {
                // stoull with base 0 auto-detects 0x for Hex
                return static_cast<T>(std::stoull(val, nullptr, 0));
            } else {
                T res;
                std::stringstream ss(val);
                if (!(ss >> res)) throw std::runtime_error("Conversion failed");
                return res;
            }
        } catch (...) {
            throw std::runtime_error("Error: Value '" + val + "' is invalid for " + std::string(name));
        }
    }

    std::map<std::string, Option> registry;
    std::vector<Option> unique_options;
    std::map<std::string, std::vector<std::string>> results;
};

}
