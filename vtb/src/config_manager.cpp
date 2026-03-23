#include "config_manager.h"
#include <iomanip>
#include <sstream>

namespace vtb {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    m_parser.add_argument("--help",     "-h", "Show this help menu", false, "false");
    m_parser.add_argument("--portmask", "-p", "Hex bitmask of ports", true, "0x0");
    m_parser.add_argument("--mode",     "-m", "polling | interrupt", false, "polling");
    m_parser.add_argument("--burst",    "-b", "Packet burst size", false, "32");
}

ConfigManager::~ConfigManager() {}

bool ConfigManager::init(int argc, char** argv) {
    try {
        m_parser.parse(argc, argv);
        if (m_parser.get<bool>("--help")) {
            m_parser.print_usage();
            return false; 
        }
        return true; 
    } catch (const std::exception& e) {
        error() << "Init Error: " << e.what();
        m_parser.print_usage();
        return false;
    }
}

void ConfigManager::dumpConfig() {
    std::lock_guard<std::mutex> lock(m_db_mutex);
    info() << "--- [Config DB Dump] ---";
    for (const auto& [key, val] : m_database) {
        std::string v = "[Object]";
        if (val.type() == typeid(int)) v = std::to_string(std::any_cast<int>(val));
        else if (val.type() == typeid(bool)) v = std::any_cast<bool>(val) ? "true" : "false";
        else if (val.type() == typeid(std::string)) v = std::any_cast<std::string>(val);
        else if (val.type() == typeid(uint64_t)) {
            std::stringstream ss; ss << "0x" << std::hex << std::any_cast<uint64_t>(val);
            v = ss.str();
        }
        details() << "Key: " << std::left << std::setw(20) << key << " | Value: " << v;
    }
}

} // namespace vtb
