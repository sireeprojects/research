#pragma once

#include <thread>
#include <atomic>

namespace vtb {

/**
 * @brief Base class for managing network port lifecycles and event monitoring.
 */
class PortController {
public:
    PortController();
    virtual ~PortController();

    // Disable copying to maintain unique ownership of the thread
    PortController(const PortController&) = delete;
    PortController& operator=(const PortController&) = delete;

protected:
    /**
     * @brief Initializes socket file descriptors. Marked virtual for overriding.
     */
    virtual void createSocketFds();

    /**
     * @brief Sets up monitoring logic. Marked virtual for overriding.
     */
    virtual void monitorSocketEvents();

    /**
     * @brief The core event loop. Marked virtual for overriding.
     */
    virtual void epollWorker();

    std::atomic<bool> isRunning;
    std::thread worker;
};

} // namespace vtb
