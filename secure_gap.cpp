#include "secure_gap.h"

#include <iostream>

#include "ble_utils.h"

CSecureGap::CSecureGap(ble::BLE &ble,
					   EventQueue &event_queue,
					   const std::string &device_name,
					   PinName led_pin,
					   const mbed::Callback<void(ble::BLE &)> &on_ble_init_callback /*= nullptr*/,
					   const mbed::Callback<void(void)> &on_connected /*= nullptr*/,
					   const mbed::Callback<void(void)> &on_disconnected /*= nullptr*/)
	: CGap(ble, event_queue, device_name, led_pin, on_ble_init_callback, on_connected, on_disconnected) {}

void CSecureGap::onBLEInitCompleteHandler(BLE::InitializationCompleteCallbackContext *context) {
	ble::peripheral_privacy_configuration_t privacyConfiguration;
	ble_error_t error;
	if (context->error) {
		std::cout << "Error during the initialisation" << std::endl;
		return;
	}
	// 1. initialize security manager. Set IO capabilities to DISPLAY_ONLY
    error = _ble.securityManager().init(false,
                                        true,
                                        SecurityManager::IO_CAPS_DISPLAY_ONLY,
                                        nullptr,
                                        false);
	
	std::cout << bleErrorToString(error, "_ble.securityManager().init() ") << std::endl;
	if (error != BLE_ERROR_NONE) {
		return;
	}
	// 2. enable legacy pairing
    error = _ble.securityManager().allowLegacyPairing(true);

	
	std::cout << bleErrorToString(error, "SecurityManager.allowLegacyPairing() ") << std::endl;
	if (error != BLE_ERROR_NONE) {
		return;
	}
	// 3. set the event handler to this object
	_ble.securityManager().setSecurityManagerEventHandler(this);
	// 4. set pairing request authorization
	error = _ble.securityManager().setPairingRequestAuthorisation(true);	
	std::cout << bleErrorToString(error, "SecurityManager::setPairingRequestAuthorisation() ") << std::endl;
	if (error != BLE_ERROR_NONE) {
		return;
	}
	// 5. Configure privacy settings
	privacyConfiguration.use_non_resolvable_random_address = false;
	privacyConfiguration.resolution_strategy =
		ble::peripheral_privacy_configuration_t::REJECT_NON_RESOLVED_ADDRESS;
	error = _ble.gap().setPeripheralPrivacyConfiguration(&privacyConfiguration);

	// 6. start advertisement using CGAP instance of this class
	CGap::onBLEInitCompleteHandler(context);

	// 7. enable privacy
	error = _ble.gap().enablePrivacy(true);
	std::cout << bleErrorToString(error, "GAP::enablePrivacy() ") << std::endl;
	if (error != BLE_ERROR_NONE) {
		return;
	}
}

void CSecureGap::pairingRequest(ble::connection_handle_t connectionHandle) {
	std::cout << "Pairing requested - authorizing" << std::endl;
	// accept the request
	ble_error_t error = _ble.securityManager().acceptPairingRequest(connectionHandle);
}

void CSecureGap::linkEncryptionResult(ble::connection_handle_t connectionHandle,
									  ble::link_encryption_t result) {
    switch(result.value()) {
        case ble::link_encryption_t::NOT_ENCRYPTED:
            std::cout << "NOT_ENCRYPTED" << std::endl;
            break;
        case ble::link_encryption_t::ENCRYPTION_IN_PROGRESS:
            std::cout << "ENCRYPTION_IN_PROGRESS" << std::endl;
            break;
        case ble::link_encryption_t::ENCRYPTED:
            std::cout << "ENCRYPTED" << std::endl;
            break;
        case ble::link_encryption_t::ENCRYPTED_WITH_MITM:
            std::cout << "ENCRYPTED_WITH_MITM" << std::endl;
            break;
        case ble::link_encryption_t::ENCRYPTED_WITH_SC_AND_MITM:
            std::cout << "ENCRYPTED_WITH_SC_AND_MITM" << std::endl;
            break;
        default:
            std::cout << "Unknown encryption state: " << result.value() << std::endl;
    }
	// print the result by printing human strings for values in ble::link_encryption_t enumeration
	
}

void CSecureGap::passkeyDisplay(ble::connection_handle_t connectionHandle,
								const SecurityManager::Passkey_t passkey) {
	// TODO:: implement this function
	// use passKeyToString() function declared in ble_utils.h file
	// to display the pass key on output stream (serial port)
    std::string key = passKeyToString(passkey);
    std::cout << "Passkey: " << key << std::endl;
	
}

void CSecureGap::confirmationRequest(ble::connection_handle_t connectionHandle) {
	std::cout << "Confirmation requested!" << std::endl;
}

void CSecureGap::passkeyRequest(ble::connection_handle_t connectionHandle) {
	std::cout << "Passkey requested!" << std::endl;
}

void CSecureGap::keypressNotification(ble::connection_handle_t connectionHandle,
									  SecurityManager::Keypress_t keypress) {
	std::cout << "keypressNotification" << std::endl;
}

void CSecureGap::signingKey(ble::connection_handle_t connectionHandle,
							const ble::csrk_t *csrk,
							bool authenticated) {
	std::cout << "signingKey" << std::endl;
}

void CSecureGap::pairingResult(ble::connection_handle_t connectionHandle,
							   SecurityManager::SecurityCompletionStatus_t result) {
	if (result == SecurityManager::SEC_STATUS_SUCCESS) {
		std::cout << "Security success" << std::endl;
	} else {
		std::cout << "Security failed" << std::endl;
	}
}

void CSecureGap::onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
	// TODO:: implement this function
	ble_error_t error;
	// print the connection status here!
	std::cout << bleErrorToString(event.getStatus(), "GAP::OnConnectionComplete()") << std::endl;
	// 1. Check that the status is successful.
	//    a. If not, just return
	//    b. If it is successful, continue with the other steps

	if (error != BLE_ERROR_NONE) {
        return;
    }


	// 2. Set the link security for the SecurityManager instance of system BLE instance
	//    a. Get the connection handle
	//    b. Set the link security to SECURITY_MODE_ENCRYPTION_WITH_MITM
	//    c. Get the return code and print it
    ble::connection_handle_t connectionHandle = event.getConnectionHandle();
    error = _ble.securityManager().setLinkSecurity(connectionHandle, ble::SecurityManager::SECURITY_MODE_ENCRYPTION_WITH_MITM);
	
	std::cout << bleErrorToString(error, "SecurityManager::setLinkSecurity()");

    CGap::onConnectionComplete(event);  

}
