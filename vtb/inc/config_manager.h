#pragma once

#include "cmdline_parser.h"
#include "messenger.h"
#include "common.h"

#include <unordered_map>
#include <map>
#include <any>
#include <mutex>
#include <optional>

namespace vtb {

class ConfigManager {
public:
    static ConfigManager& getInstance();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    bool init(int argc, char** argv);
    
    template<typename T>
    T getArg(std::string_view name) const { return m_parser.get<T>(name); }

    template<typename T>
    void setValue(const std::string& key, T&& value) {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        m_database[key] = std::make_any<typename std::decay<T>::type>(std::forward<T>(value));
    }

    template<typename T>
    std::optional<T> getValue(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_db_mutex);
        auto it = m_database.find(key);
        if (it != m_database.end()) {
            try { return std::any_cast<T>(it->second); } catch (...) {}
        }
        return std::nullopt;
    }

    void dumpConfig();

private:
    ConfigManager();
    ~ConfigManager();

    CmdlineParser m_parser;
    std::unordered_map<std::string, std::any> m_database;
    mutable std::mutex m_db_mutex;

    std::map<int, struct portmap> pmap;
};

} // namespace vtb
