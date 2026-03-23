#include "config_manager.h"
#include <iomanip>
#include <sstream>

namespace vtb {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    // Registry for CLI Arguments
    m_parser.add_argument("--portmask", "-p", "Hexadecimal bitmask", true, "0x0");
    m_parser.add_argument("--mode",     "-m", "polling|interrupt", false, "polling");
    m_parser.add_argument("--burst",    "-b", "Packet burst size", false, "32");
    m_parser.add_argument("--promisc",  "-P", "Promiscuous mode", false, "false");
}

ConfigManager::~ConfigManager() {}

bool ConfigManager::init(int argc, char** argv) {
    try {
        m_parser.parse(argc, argv);
        return true; 
    } catch (const std::exception& e) {
        error() << "ConfigManager Init failed: " << e.what();
        return false;
    }
}

void ConfigManager::dumpConfig() {
    std::lock_guard<std::mutex> lock(m_db_mutex);
    info() << "--- [ConfigManager Database Dump] ---";
    for (const auto& [key, val] : m_database) {
        std::string v_str = "[Object]";
        if (val.type() == typeid(int)) v_str = std::to_string(std::any_cast<int>(val));
        else if (val.type() == typeid(bool)) v_str = std::any_cast<bool>(val) ? "true" : "false";
        else if (val.type() == typeid(std::string)) v_str = std::any_cast<std::string>(val);
        else if (val.type() == typeid(uint64_t)) {
            std::stringstream ss; ss << "0x" << std::hex << std::any_cast<uint64_t>(val);
            v_str = ss.str();
        }
        details() << "Key: " << std::left << std::setw(20) << key << " | Value: " << v_str;
    }
}

} // namespace vtb
