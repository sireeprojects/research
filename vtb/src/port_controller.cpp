#include "port_controller.h"

namespace vtb {

PortController::PortController() : isRunning(true) {
    // Start the worker thread immediately upon construction
    worker = std::thread(&PortController::epollWorker, this);
}

PortController::~PortController() {
    isRunning = false;
    if (worker.joinable()) {
        worker.join();
    }
}

void PortController::createSocketFds() {
    // Empty base definition
}

void PortController::monitorSocketEvents() {
    // Empty base definition
}

void PortController::epollWorker() {
    while (isRunning) {
        // Base event loop logic
    }
}

} // namespace vtb
