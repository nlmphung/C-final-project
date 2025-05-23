#pragma once
#include "gap.h"

/**
 * @brief This class extends the `CGap` class to incorporate Bluetooth Low Energy (BLE) security features, managing 
 * secure connections and pairing procedures. It handles various security-related events and provides methods for 
 * managing passkeys and encryption. It inherits from both `CGap` and `ble::SecurityManager::EventHandler`.
 *
 * @details The `CSecureGap` class builds upon the functionality of `CGap` by adding security features crucial for 
 * establishing and maintaining secure BLE connections. Key responsibilities include:
 *
 *   - **Security Manager Initialization:** Initializes the BLE Security Manager to handle secure connections.
 *   - **Pairing Request Handling:** Manages incoming pairing requests, allowing the application to accept or reject 
 *     them. This involves interaction with the user through passkey entry or confirmation.
 *   - **Encryption Management:** Handles link encryption establishment and management, ensuring secure data 
 *     transmission.
 *   - **Passkey Management:** Provides methods for displaying and handling passkeys during the pairing process. 
 *     Supports various input methods (keyboard, buttons).
 *   - **Event Handling:** Implements the `ble::SecurityManager::EventHandler` interface to respond to various 
 *     security-related events, including pairing requests, encryption changes, and keypresses.
 *   - **Security-Related Callbacks:** Allows setting callbacks to respond to connection events considering the 
 *     security state.
 *
 * This class provides a comprehensive approach to integrating security into BLE applications, simplifying the 
 * development of secure and reliable BLE-based systems.
 *
 * Example usage:
 * @code
 * ble::BLE &ble = BLE::Instance();
 * EventQueue event_queue;
 * CSecureGap secure_gap(ble, event_queue, "DeviceName", LED1,
 *                       [](ble::BLE &ble) { // BLE init callback },
 *                       []() { // Connected callback  },
 *                       []() { // Disconnected callback  });
 * @endcode
 */
class CSecureGap : public CGap, public ble::SecurityManager::EventHandler {
   private:
   protected:
	/**
	 * @brief This the application's system BLE initialization complete function.
	 *
	 * @note This function must be registered as BLE initialization completion handler function
	 * by calling BLE::init member function of BLE class.
	 *
	 * @note There are different overloads of BLE::init
	 *
	 * @note You must call _on_ble_init_callback function if it is not nullptr
	 *
	 * @param context The initialization context
	 */
	virtual void onBLEInitCompleteHandler(BLE::InitializationCompleteCallbackContext *context) override;

   public:
	CSecureGap() = delete;
	CSecureGap(const CSecureGap &) = delete;
	CSecureGap &operator=(const CSecureGap &) = delete;
	/**
	 * @brief Construct a new CSecureGap object
	 *
	 * @param ble A reference to the system's BLE object
	 * @param event_queue A reference to the application's event queue object
	 * @param device_name The name of the device
	 * @param led_pin The LED pin name that will be used by the application
	 * @param on_ble_init_callback The callback function to be called when BLE initialization completes.
	 * @param on_connected The callback function to be called when a peer is connected
	 * @param on_disconnected The callback function to be called when a peer is disconnected
	 */
	CSecureGap(ble::BLE &ble,
			   EventQueue &event_queue,
			   const std::string &device_name,
			   PinName led_pin,
			   const mbed::Callback<void(ble::BLE &)> &on_ble_init_callback = nullptr,
			   const mbed::Callback<void(void)> &on_connected = nullptr,
			   const mbed::Callback<void(void)> &on_disconnected = nullptr);
	/**
	 * \brief Called when a pairing request is received. Application should respond by
	 *        calling the appropriate function: acceptPairingRequest() or cancelPairingRequest().
	 *        This event will only trigger if setPairingRequestAuthorisation() was called with true.
	 *        Otherwise the stack will handle the requests.
	 *
	 * \param[in] connectionHandle connection handle
	 *
	 */
	virtual void pairingRequest(ble::connection_handle_t connectionHandle) override;

	/**
	 * \brief Triggered by change of encryption state on a link. This change can be initiated
	 * locally or by the remote peer.
	 *
	 * \param connectionHandle connection handle
	 * \param result encryption state of the link
	 */
	virtual void linkEncryptionResult(ble::connection_handle_t connectionHandle,
									  ble::link_encryption_t result) override;

	/**
	 * \brief Triggered during pairing based on IO capabilities of devices. Display the given
	 *        passkey on the local device for user verification.
	 *
	 * \param connectionHandle connection handle
	 * \param passkey 6 digit passkey to be displayed
	 */
	virtual void passkeyDisplay(ble::connection_handle_t connectionHandle,
								const SecurityManager::Passkey_t passkey) override;

	/**
	 * \brief Indicates to the application that a confirmation is required. This is used
	 *        when the device does not have a keyboard but has a yes/no button. The device
	 *        displays numbers on its display in response to passkeyDisplay and the user
	 *        checks if they are the same on both devices. The application should proceed
	 *        by supplying the confirmation using the confirmationEntered function.
	 *
	 * \param connectionHandle connection handle
	 */
	virtual void confirmationRequest(ble::connection_handle_t connectionHandle) override;

	/**
	 * \brief Indicates to the application that a passkey is required. The application should
	 *        proceed by supplying the passkey through the passkeyEntered function.
	 *
	 * \param connectionHandle connection handle
	 */
	virtual void passkeyRequest(ble::connection_handle_t connectionHandle) override;

	/**
	 * \brief Notifies the application that a key was pressed by the peer during passkey entry.
	 *        This is only informative to provide feedback to the user. These events will only
	 *        be triggered if you call setKeypressNotification()
	 *
	 * \param connectionHandle connection handle
	 * \param keypress type of keypress event
	 */
	virtual void keypressNotification(ble::connection_handle_t connectionHandle,
									  SecurityManager::Keypress_t keypress) override;

	/**
	 * \brief Delivers the signing key to the application.
	 *
	 * \param connectionHandle connection handle
	 * \param csrk signing key, pointer only valid during call
	 * \param authenticated indicates if the signing key is authenticated
	 */
	virtual void signingKey(ble::connection_handle_t connectionHandle,
							const ble::csrk_t *csrk,
							bool authenticated) override;

	/**
	 * \brief Indicates to the application that pairing has completed.
	 *
	 * \param connectionHandle connection handle
	 * \param result result of the pairing indicating success or reason for failure
	 */
	virtual void pairingResult(ble::connection_handle_t connectionHandle,
							   SecurityManager::SecurityCompletionStatus_t result) override;

	/**
	 * @brief Handler function called when the BLE connection process completes.
	 *
	 * @note You are required to stop blinking the application LED when BLE is connected.
	 * You can use EventQueue::cancel function with the event identifier to stop periodically
	 * LED toggling.
	 *
	 * @param event The connection event object
	 */
	virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;
};
