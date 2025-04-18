#pragma once
#include <BLE.h>
#include <mbed.h>

#include <memory>
#include <vector>

#include "gatt_service.h"

/**
 * @brief Implements the Immediate Alert Service (IAS) as defined in the Bluetooth Specification.
 *
 * This service allows a client to control the alert level on the device. The alert level is
 * reflected by the intensity of a connected LED. See the Bluetooth SIG specification for details
 * on the Immediate Alert Service: [https://www.bluetooth.com/specifications/specs/immediate-alert-service-1-0/](https://www.bluetooth.com/specifications/specs/immediate-alert-service-1-0/)
 *
 * @details This service provides a single characteristic, `GattCharacteristic::UUID_ALERT_LEVEL_CHAR`,
 * which represents the current alert level.  This characteristic is writable without response, allowing
 * a client to update the alert level asynchronously.  The server (this implementation) responds to 
 * changes in the alert level by adjusting the duty cycle of a PWM-controlled LED.  This creates a visual
 * representation of the alert level on the device.
 *
 * The alert level is represented by the `AlertLevel` enum.  The following mappings exist between the enum values and the LED behavior:
 * - `IAS_ALERT_LEVEL_NO_ALERT`: LED is OFF (100% duty cycle).
 * - `IAS_ALERT_LEVEL_MEDIUM`: LED is at approximately 50% duty cycle.
 * - `IAS_ALERT_LEVEL_HIGH`: LED is ON (0% duty cycle).
 *
 * Upon connection, the alert level is reset to `IAS_ALERT_LEVEL_NO_ALERT`. Upon disconnection, the 
 * alert level is set to `IAS_ALERT_LEVEL_MEDIUM`. These default behaviors can be modified by overriding the
 * `onConnected` and `onDisconnected` methods.
 *
 * @note The UUID of this service is `GattService::UUID_IMMEDIATE_ALERT_SERVICE`.  The security requirements for reading, writing,
 * and updating the Alert Level characteristic are defined within the constructor.
 */
class CImmediateAlertService : public CGattService {
public:
    /**
     * @brief Enumerates the possible alert levels.  These values correspond to the
     * alert levels defined in the Bluetooth specification for the Immediate Alert Service.
     */
    enum AlertLevel { 
        IAS_ALERT_LEVEL_NO_ALERT = 0,  ///< No alert. LED is OFF.
        IAS_ALERT_LEVEL_MEDIUM = 1,   ///< Medium alert. LED is at approximately 50% duty cycle.
        IAS_ALERT_LEVEL_HIGH = 2     ///< High alert. LED is ON (100% duty cycle).
    };

private:
    PwmOut _led;       ///< PWM output for controlling the LED intensity.
    AlertLevel _alert_level; ///< The current alert level.
    uint8_t _alert_level_value; ///< The numeric value of the current alert level.

public:
    /**
     * @brief Deleted default constructor.  Use the parameterized constructor instead.
     */
    CImmediateAlertService() = delete;
    /**
     * @brief Deleted copy constructor.
     */
    CImmediateAlertService(const CImmediateAlertService &) = delete;
    /**
     * @brief Deleted assignment operator.
     */
    const CImmediateAlertService &operator=(const CImmediateAlertService &) = delete;

    /**
     * @brief Constructs a new CImmediateAlertService object.
     * 
     * @param led_pin The pin connected to the LED that will visually represent the alert level.
     *                The LED must be capable of PWM control.
     * @note The constructor initializes the LED PWM period to 1 second and creates the alert level characteristic.
     */
    CImmediateAlertService(PinName led_pin);
    /**
     * @brief Destroys the CImmediateAlertService object.  Releases resources.
     */
    ~CImmediateAlertService();

    // = delete; -> Prevents exposing the base class member functions that aren't overridden here.
    ble_error_t createService(const char *uuid) = delete;

    /**
     * @brief Handles data written to the alert level characteristic.
     * 
     * Updates the alert level and adjusts the LED's PWM duty cycle accordingly.
     *
     * @param characteristic The characteristic that was written to (should be the alert level characteristic).
     * @param data A pointer to the data that was written.  Should be a single byte representing the alert level.
     * @param size The size of the data written (should be 1).
     */
    virtual void onDataWrittenHandler(GattCharacteristic *characteristic, const uint8_t *data, uint16_t size) final;

    /**
     * @brief Handler called when a peer device connects.
     * 
     * Resets the alert level to `IAS_ALERT_LEVEL_NO_ALERT` and updates the characteristic value.
     */
    virtual void onConnected() final;
    /**
     * @brief Handler called when a peer device disconnects.
     * 
     * Sets the alert level to `IAS_ALERT_LEVEL_MEDIUM` and updates the characteristic value.
     */
    virtual void onDisconnected() final;

    /**
     * @brief Registers the service with the BLE stack.
     *
     * @param ble A reference to the system's BLE instance.
     */
    virtual void registerService(ble::BLE &ble) final;
};
