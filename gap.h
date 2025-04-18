#pragma once
#include <BLE.h>
#include <mbed.h>

#include <string>

/**
 * @brief This class implements the Generic Access Profile (GAP) functionality for a Bluetooth Low Energy (BLE) device. 
 * It handles device advertising, connection management, and disconnection events, providing a foundation for BLE 
 * communication. The class is designed to be inherited from, allowing for customization and extension.
 *
 * @details The `CGap` class manages the core aspects of BLE device discovery and connection. Key features include:
 *
 *   - **Device Advertising:** Configures and starts BLE advertising, making the device discoverable by other BLE 
 *     devices. The advertisement data can be customized, allowing the device to broadcast specific information.
 *   - **Connection Management:** Handles incoming connection requests, establishes secure connections, and manages the 
 *     connection lifecycle. It provides callback mechanisms to notify the application of connection and disconnection 
 *     events.
 *   - **Disconnection Handling:** Gracefully handles disconnections, performing any necessary cleanup actions.
 *   - **LED Indication:** Uses an LED to provide visual feedback on the device's status (advertising, connected, 
 *     disconnected). This aids in debugging and monitoring.
 *   - **Asynchronous Operations:** Uses an event queue to handle asynchronous BLE events, preventing blocking operations 
 *     and improving responsiveness.
 *   - **Extensibility:** The class is designed to be inherited from. Derived classes can override virtual methods to 
 *     customize behavior and add additional functionality.
 *   - **Security:** The class incorporates security mechanisms, allowing it to handle secure connection procedures.
 *
 * The `CGap` class utilizes the Mbed OS BLE library to interact with the BLE hardware and manages BLE events using the 
 * `ble::Gap::EventHandler` interface. It leverages an `EventQueue` for efficient event processing and provides callback 
 * mechanisms to notify the application of key events, such as BLE initialization completion, connection establishment, 
 * and disconnection.
 *
 * @note You must inherit this class as specified in the task description. This class is non-copyable.
 *
 * @note You can print errors of `ble_error_t` type using the static member function `ble::BLE::errorToString` of the 
 * `ble::BLE` class. This function returns a string; you can use the returned string at your convenience.
 *
 * -- You can also use the `bleErrorToString` function declared in `ble_utils.h`.
 */
class CGap : private mbed::NonCopyable<CGap>, public ble::Gap::EventHandler {
protected:
    ble::BLE &_ble;             //!< Reference to the BLE system's BLE object instance
    EventQueue &_event_queue;   //!< Reference to the application's EventQueue
    std::string _device_name;   //!< The name of the device

    DigitalOut _led;            //!< System LED for indicating advertisement and connection status
    int _led_event_id;          //!< Event identifier (returned by `EventQueue::call_every` function)

    ble::AdvertisingDataBuilder _adv_data_builder; //!< Advertisement data builder for creating, storing, and 
                                                   //!< manipulating advertisement data
    uint8_t _adv_data_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE]; //!< Buffer used by the advertisement data builder

    // Callback functions for BLE events
    mbed::Callback<void(ble::BLE &)> _on_ble_init_callback;
    mbed::Callback<void(void)> on_connected;
    mbed::Callback<void(void)> on_disconnected;


protected:
    /**
     * @brief Toggles the LED state.
     *
     * @note This function must be called by the EventQueue dispatcher.
     */
    void toggleLED();

    /**
     * @brief Updates the advertisement data (if needed).
     *
     * @note This function is not used in this task.
     */
    void updateAdvertisementData();

    /**
     * @brief Handler for BLE initialization completion.
     *
     * @param context The initialization context.
     * @note This function must be registered as the BLE initialization completion handler by calling `BLE::init`.
     * @note Calls the user-provided `_on_ble_init_callback` if not nullptr.
     */
    virtual void onBLEInitCompleteHandler(BLE::InitializationCompleteCallbackContext *context);

    /**
     * @brief Schedules BLE events for processing in the EventQueue.
     *
     * @param context The context of the events to be processed.
     * @note Binds the event queue to process BLE events using `BLE::processEvents`.
     */
    void scheduleBLEEventsToProcess(BLE::OnEventsToProcessCallbackContext *context);

    /**
     * @brief Handler for connection completion events. Stops LED blinking when connected.
     *
     * @param event The connection event object.
     */
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    /**
     * @brief Handler for disconnection completion events.
     *
     * @param event The disconnection event object.
     */
    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

public:
    CGap() = delete;
    CGap(const CGap &) = delete;
    const CGap &operator=(const CGap &) = delete;

    /**
     * @brief Constructs a new CGap object.
     *
     * @param ble Reference to the system's BLE object.
     * @param event_queue Reference to the application's event queue.
     * @param device_name The name of the device.
     * @param led_pin The LED pin name to use.
     * @param on_ble_init_callback Callback for BLE init completion.
     * @param on_connected Callback for connection events.
     * @param on_disconnected Callback for disconnection events.
     */
	CGap(ble::BLE &ble,
		 EventQueue &event_queue,
		 const std::string &device_name,
		 PinName led_pin,
		 const mbed::Callback<void(ble::BLE &)> &on_ble_init_callback = nullptr,
		 const mbed::Callback<void(void)> &on_connected = nullptr,
		 const mbed::Callback<void(void)> &on_disconnected = nullptr);

    /**
     * @brief Starts the BLE operation. Registers event handlers, initializes BLE, and starts the event queue dispatcher.
     * 
	 * It performs the following operations:
	 * 1. Registers this object's member function as BLE::onEventsToProcess function
	 *    In order to create FunctionPointerWithContext object (required by BLE::onEventsToProcess)
	 *    you can use makeFunctionPointer function.
	 * 2. Register GAP event handler as this object since this class is
	 *    inherited from ble::Gap::EventHandler structure.
	 * 3. Initialize system's BLE object, and register this object's member function
	 *    to be called when the BLE initialization completes.
	 * 4. Dispatches EventQueue events for ever.
	 *
	 * @note It is useful to print the the status of this operations for debugging
	 * and monitoring purposes.
     */
    void run();

    /**
     * @brief Starts device advertising. Configures and starts advertising, including setting the advertisement data and 
     * parameters. Starts LED blinking to indicate advertising.
     *
	 * Advertisement is configured and started as follows:
	 * 1. Create advertisement data using ble::AdvertisingDataBuilder type member
	 * 2. Setup the advertisement parameters
	 * 3. Set the advertisement payload
	 * 4. Start the advertisement
	 * 5. Start toggling the application LED
	 *
	 * @note It is useful to print the the status of this operations for debugging
	 * and monitoring purposes.
     */
    void startAdvertising();

    /**
     * @brief Sets the callback function for BLE initialization completion.
     *
     * @param callback The callback function.
     */
    void setOnBLEInitCompleteCallback(const mbed::Callback<void(ble::BLE &)> &callback);

    /**
     * @brief Sets the callback function for connection events.
     *
     * @param callback The callback function.
     */
    void setOnConnectedCallback(const mbed::Callback<void(void)> &callback);

    /**
     * @brief Sets the callback function for disconnection events.
     *
     * @param callback The callback function.
     */
    void setOnDisconnectedCallback(const mbed::Callback<void(void)> &callback);
};
