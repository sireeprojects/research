#include "tester_sockets_controller.h"

namespace vtb {

TesterSocketsController::TesterSocketsController() : PortController() {
    // Calling protected start() ensures Tester overrides are used
    start();
}

void TesterSocketsController::createSocketFds() {
    // Test-specific socket initialization
}

void TesterSocketsController::monitorSocketEvents() {
    // Test-specific epoll registration
}

void TesterSocketsController::epollWorker() {
    while (isRunning) {
        // Test-specific packet processing or simulation logic
    }
}

} // namespace vtb
