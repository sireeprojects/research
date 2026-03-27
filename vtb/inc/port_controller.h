#pragma once

#include <thread>
#include <atomic>

namespace vtb {

/**
 * @brief Base class for managing network port lifecycles.
 */
class PortController {
public:
    PortController();
    
    /**
     * @brief Virtual destructor ensures derived cleanup.
     */
    virtual ~PortController();

    // Prevent copying to maintain unique ownership of the thread
    PortController(const PortController&) = delete;
    PortController& operator=(const PortController&) = delete;

protected:
    /**
     * @brief Orchestrates the initialization sequence. 
     * Protected to prevent external calls while allowing derived overrides.
     */
    virtual void start();

    /**
     * @brief Initializes socket file descriptors.
     */
    virtual void createSocketFds();

    /**
     * @brief Sets up monitoring logic (e.g., epoll_ctl).
     */
    virtual void monitorSocketEvents();

    /**
     * @brief The core event loop logic.
     */
    virtual void epollWorker();

    std::atomic<bool> isRunning;
    std::thread worker;

private:
    /**
     * @brief Internal helper to launch the epollWorker thread.
     */
    void startEpoll();
};

} // namespace vtb
