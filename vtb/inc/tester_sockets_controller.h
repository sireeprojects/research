#pragma once

#include "port_controller.h"

namespace vtb {

class TesterSocketsController : public PortController {
public:
    /**
     * @brief Constructor calls start() to initiate test-specific logic.
     */
    TesterSocketsController();
    virtual ~TesterSocketsController() = default;

protected:
    void createSocketFds() override;
    void monitorSocketEvents() override;
    void epollWorker() override;
};

} // namespace vtb
