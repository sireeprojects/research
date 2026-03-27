#include "port_controller.h"

namespace vtb {

PortController::PortController() : isRunning(false) {
    // Constructor remains lean.
}

PortController::~PortController() {
    isRunning = false;
    if (worker.joinable()) {
        worker.join();
    }
}

void PortController::start() {
    // Orchestration logic - can be overridden by specialized controllers
    createSocketFds();
    monitorSocketEvents();
    startEpoll();
}

void PortController::startEpoll() {
    if (!isRunning) {
        isRunning = true;
        worker = std::thread(&PortController::epollWorker, this);
    }
}

void PortController::createSocketFds() {
    // Default base implementation
}

void PortController::monitorSocketEvents() {
    // Default base implementation
}

void PortController::epollWorker() {
    while (isRunning) {
        // Base event loop logic
    }
}

} // namespace vtb
