#include "port_controller.h"
#include "cmdline_parser.h"
#include "config_manager.h"
#include "port_handler.h"
#include "vhost_controller.h"

#include "config_manager.h"
#include "logger.h"
#include "messenger.h"
#include <vector>

// A custom structure to demonstrate complex type storage
struct PortStats {
    uint64_t rx_packets;
    uint64_t tx_packets;
    double error_rate;
};

int main(int argc, char** argv) {
    // 1. Setup Logger
    vtb::Logger::getInstance().init("dpdk_app.log", vtb::LogLevel::FULL);
    vtb::info() << "Starting DPDK Application Environment...";

    // 2. Initialize ConfigManager and parse CLI
    auto& config = vtb::ConfigManager::getInstance();
    if (!config.init(argc, argv)) {
        vtb::error() << "Invalid command line arguments.";
        return -1;
    }

    // 3. Demonstrate retrieval of CLI parameters
    try {
        uint64_t pmask = config.getArg<uint64_t>("--portmask");
        std::string mode = config.getArg<std::string>("--mode");
        int burst = config.getArg<int>("--burst");
        bool promisc = config.getArg<bool>("--promisc");

        vtb::info() << "CLI: Portmask=" << std::hex << pmask << " Mode=" << mode
                   << " Burst=" << std::dec << burst << " Promisc=" << (promisc ? "ON" : "OFF");
    } catch (...) {
        vtb::error() << "Failed to fetch one or more CLI arguments.";
    }

    // 4. Demonstrate "Database" functionality with all basic types
    vtb::info() << "Populating Internal Config Database...";

    // Integer / Unsigned types
    config.setValue("max_lcores", 16);
    config.setValue("socket_id", 0u);

    // Boolean
    config.setValue("hugepages_active", true);

    // String (using ""s literal for std::string)
    using namespace std::string_literals;
    config.setValue("driver_name", "net_virtio"s);

    // Floating Point
    config.setValue("timeout_ms", 500.5);

    // 5. Demonstrate Complex/Custom Types
    PortStats eth0_stats = { 1500000, 1200000, 0.001 };
    config.setValue("stats_port_0", eth0_stats);

    // 6. Retrieval Tests
    vtb::info() << "Performing Retrieval Tests...";

    // Successful retrieval with value_or
    auto lcores = config.getValue<int>("max_lcores").value_or(1);
    vtb::info() << "Retrieved lcores: " << lcores;

    // Type mismatch protection test
    auto failed_cast = config.getValue<std::string>("max_lcores"); // Trying to get int as string
    if (!failed_cast) {
        vtb::details() << "Safety Check: Correctly blocked invalid type cast for 'max_lcores'.";
    }

    // Custom struct retrieval
    auto stats = config.getValue<PortStats>("stats_port_0");
    if (stats) {
        vtb::info() << "Port 0 Stats - RX: " << stats->rx_packets << " Error Rate: " << stats->error_rate;
    }

    // 7. Final Debug Dump (Shows everything in the log)
    config.dumpConfig();

    vtb::info() << "Application Setup Complete.";
    return 0;
}
