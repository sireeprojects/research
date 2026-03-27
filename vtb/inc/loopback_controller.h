#pragma once

#include "port_controller.h"

namespace vtb {

class LoopbackController : public PortController {
public:
    /**
     * @brief Constructor calls start() to initiate loopback-specific logic.
     */
    LoopbackController();
    virtual ~LoopbackController() = default;

protected:
    void createSocketFds() override;
    void monitorSocketEvents() override;
    void epollWorker() override;
};

} // namespace vtb
