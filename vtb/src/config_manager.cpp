#include "config_manager.h"
#include <iomanip>

namespace vtb {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    // Fixed: Using add_argument (snake_case) as per your compiler error
    m_parser.add_argument("--portmask", "-p", "Hexadecimal bitmask of ports", true);
    m_parser.add_argument("--mode", "-m", "Application mode", false, "polling");
}

ConfigManager::~ConfigManager() {}

bool ConfigManager::init(int argc, char** argv) {
    try {
        // Fixed: parse returns void, so we call it and then return true
        m_parser.parse(argc, argv);
        return true; 
    } catch (const std::exception& e) {
        error() << "ConfigManager: Parsing failed: " << e.what();
        return false;
    }
}

void ConfigManager::dumpConfig() {
    std::lock_guard<std::mutex> lock(m_db_mutex);
    info() << "--- [ConfigManager Database Dump] ---";
    
    for (const auto& [key, val] : m_database) {
        std::string value_str = "[Complex Type]";

        if (val.type() == typeid(int)) 
            value_str = std::to_string(std::any_cast<int>(val));
        else if (val.type() == typeid(bool))
            value_str = std::any_cast<bool>(val) ? "true" : "false";
        else if (val.type() == typeid(std::string))
            value_str = std::any_cast<std::string>(val);
        else if (val.type() == typeid(uint64_t))
            value_str = "0x" + std::to_string(std::any_cast<uint64_t>(val));

        details() << "Key: " << std::left << std::setw(20) << key 
                  << " | Value: " << value_str;
    }
    info() << "-------------------------------------";
}

} // namespace vtb
