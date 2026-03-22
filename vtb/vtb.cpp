#include "avip_controller.h"
#include "cmdline_parser.h"
#include "config_manager.h"
#include "port_handler.h"
#include "vhost_controller.h"

#include "messenger.h"

int main() {
    vtb::Logger::getInstance().init("session.log", vtb::LogLevel::FULL);

    int c1 = 100;

    // Direct and simple
    vtb::info() << "System Bootstrapped.";
    vtb::info() << "Counter value: " << c1;
    vtb::error() << "Failed to initialize Link " << 0;

    // Only shows in FULL mode
    vtb::details() << "Memory address: " << &c1;

    return 0;
}
