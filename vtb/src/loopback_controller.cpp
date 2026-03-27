#include "loopback_controller.h"

namespace vtb {

LoopbackController::LoopbackController() : PortController() {
    // Calling protected start() ensures Loopback overrides are used
    start();
}

void LoopbackController::createSocketFds() {
    // Loopback-specific socket initialization
}

void LoopbackController::monitorSocketEvents() {
    // Loopback-specific epoll registration
}

void LoopbackController::epollWorker() {
    while (isRunning) {
        // Loopback-specific packet processing
    }
}

} // namespace vtb
