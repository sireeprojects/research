#pragma once

#include "port_controller.h"

namespace vtb {

/**
 * @brief Specialization of PortController for testing socket interactions.
 */
class TesterSocketsController : public PortController {
public:
    /**
     * @brief Construct a new Tester Sockets Controller object.
     */
    TesterSocketsController();

    /**
     * @brief Destroy the Tester Sockets Controller object.
     */
    virtual ~TesterSocketsController() = default;

protected:
    /**
     * @brief Overridden to initialize test-specific socket descriptors.
     */
    void createSocketFds() override;

    /**
     * @brief Overridden to monitor test-specific events.
     */
    void monitorSocketEvents() override;

    /**
     * @brief Overridden to implement a test-specific event handler.
     */
    void epollWorker() override;
};

} // namespace vtb
