#pragma once

#include <BLE.h>
#include <list>
#include <memory>
#include "gatt_service.h"

/**
 * @brief This class manages multiple GATT services, handling service registration, data read/write events, and connection
 * management. It simplifies the process of creating and managing a collection of GATT services within a Bluetooth Low
 * Energy (BLE) application. The class inherits from `GattServer::EventHandler` to handle various GATT server events.
 *
 * @details The `CGattServer` class acts as a container for GATT services, providing a centralized point for managing their
 * lifecycle and interactions with clients. Key functionalities include:
 *
 *   - **Service Management:** Allows adding and managing multiple `CGattService` objects. Services are added using the
 *     `addService` method and must be created before being added.
 *   - **Server Start/Stop:** Provides methods to start and stop the GATT server. Starting the server registers all added
 *     services with the system's GATT server and sets this object as the event handler. Stopping the server unregisters
 *     the services and removes the event handler.
 *   - **Event Handling:** Implements several event handlers to manage data read/write operations and client connection
 *     events (`onConnected`, `onDisconnected`, `onDataWritten`, `onDataRead`, `onUpdatesEnabled`, `onUpdatesDisabled`,
 *     `onConfirmationReceived`). These handlers can be overridden in derived classes to customize behavior.
 *   - **Asynchronous Operations:** The class handles GATT server events asynchronously, preventing blocking operations and
 *     improving application responsiveness.
 *   - **Data read and write events, with default behavior defined in the base class.
 *
 * The `CGattServer` class uses a `std::list` to store pointers to the managed services, allowing for dynamic addition and
 * removal of services.
 */
class CGattServer : public GattServer::EventHandler, public mbed::NonCopyable<CGattServer> {
private:
	// This time we are not required to store our managed objects in contiguous memory regions.
	// we can use std::list container which has relaxed insertion, removal properties.
    std::list<CGattService*> _services;  // The services managed by this server.
    bool _started;                       // Indicates if the server has been started.

public:
    /**
     * @brief Constructs a new CGattServer object. Initializes the server in the stopped state.
     */
    CGattServer();
    /**
     * @brief Destroys the CGattServer object. Releases resources.
     */
    ~CGattServer();
    /**
     * @brief Adds a service to this server.
     *
     * @details A service can only be added if it has been created and the server has not yet been started.
     *
     * @param service The service to be added.
     * @return true if the service was successfully added; false otherwise.
     */
    bool addService(CGattService& service);
    /**
     * @brief Starts the GATT server. Registers all added services with the system's GATT server and sets this object as the
     * event handler.
     *
     * @param ble A reference to the system's BLE instance.
     */
    void startServer(ble::BLE& ble);

    /**
     * @brief Handler called when a client connects to the server.
     */
    void onConnected();

    /**
     * @brief Handler called when a client disconnects from the server.
     */
    void onDisconnected();

    /**
     * @brief Handler called after an attribute has been written.
     *
     * @param e Event parameters.
     */
    virtual void onDataWritten(const GattWriteCallbackParams& e) override;
    /**
     * @brief Handler called after an attribute has been read.
     *
     * @param e The event parameters.
     */
    virtual void onDataRead(const GattReadCallbackParams& e) override;
    /**
     * @brief Handler called after a client subscribes to notifications or indications.
     *
     * @param params The event parameters.
     */
    virtual void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams& params) override;
    /**
     * @brief Handler called after a client unsubscribes from notifications or indications.
     *
     * @param params The event parameters.
     */
    virtual void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams& params) override;
    /**
     * @brief Handler called when an indication confirmation is received.
     *
     * @param params The event parameters.
     */
    virtual void onConfirmationReceived(const GattConfirmationReceivedCallbackParams& params) override;
};
