#pragma once

#include "port_controller.h"

namespace vtb {

/**
 * @brief Specialization of PortController for loopback interface management.
 */
class LoopbackController : public PortController {
public:
    LoopbackController();
    virtual ~LoopbackController() = default;

protected:
    /**
     * @brief Overridden to initialize loopback-specific socket descriptors.
     */
    void createSocketFds() override;

    /**
     * @brief Overridden to monitor loopback-specific events.
     */
    void monitorSocketEvents() override;

    /**
     * @brief Overridden to implement a loopback-specific event handler.
     */
    void epollWorker() override;
};

} // namespace vtb
