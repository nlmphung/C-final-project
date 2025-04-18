#include "gap.h"

#include <iostream>

#include "ble_utils.h"

// TODO: Copy your solution from Module 8.
// Note: onBLEInitCompleteHandler member is virtual now.
// Note: You can get human readable strings of ble_error_t type returns values
// using bleErrorToString function declared in ble_utils.h

void CGap::toggleLED() {
	// TODO:: Implement this functions
    bool new_state = !_led.read();
	_led.write(new_state);
	std::cout << "LED toggled to " << new_state << std::endl;
}

void CGap::updateAdvertisementData() {
	// We do not need to update advertisement data
	std::cout << "Advertisement data updated!" << std::endl;
}
void CGap::onBLEInitCompleteHandler(BLE::InitializationCompleteCallbackContext *context) {
	// TODO:: Implement this functions
	// 1. check for the initialization errors using error member of context
	ble_error_t error = context->error;
    if (error != BLE_ERROR_NONE) {
        std::cout << bleErrorToString(error);
        return;
    }

	// The BLE interface can be accessed now.
	std::cout << "BLE init completed" << std::endl;
	// 2. get and print the device address
	ble::own_address_type_t addrType;
	ble::address_t address;
	error = context->ble.gap().getAddress(addrType, address);
	std::cout << bleErrorToString(error, "GAP::getAddress()") << std::endl;
	if (error == BLE_ERROR_NONE) {
		// print the own Bluetooth address using
		// bluetoothAddressToString utility function defined in ble_utils.h
		// note that there are 3 overloads of that function.
        std::cout << "Address: " << bluetoothAddressToString(address) << std::endl;
	} else {
        std::cout << "Error with address: " << bluetoothAddressToString(address) << " " << bleErrorToString(error) << std::endl;
    }
	// 3. call _on_ble_init_callback member if it is not nullptr
	if (_on_ble_init_callback != nullptr) {
        _event_queue.call(_on_ble_init_callback, std::ref(_ble));
    }
    _led_event_id = _event_queue.call_every(500ms, this, &CGap::toggleLED);
    std::cout << "LED event scheduled, ID = " << _led_event_id << std::endl;
	// 4. start advertising
	startAdvertising();

}

void CGap::scheduleBLEEventsToProcess(BLE::OnEventsToProcessCallbackContext *context) {
	_event_queue.call(callback(&(context->ble), &BLE::processEvents));
}


CGap::CGap(ble::BLE &ble,
		   EventQueue &event_queue,
		   const std::string &device_name,
		   PinName led_pin,
           // ?? check if issues
		   const mbed::Callback<void(ble::BLE &)> &on_ble_init_callback,
           const mbed::Callback<void(void)> &on_connected,
           const mbed::Callback<void(void)> &on_disconnected)  : _ble(ble), _event_queue(event_queue), 
                                           _device_name(device_name), 
                                           _led(led_pin), 
                                           _adv_data_builder(_adv_data_buffer, sizeof(_adv_data_buffer)),
                                           _on_ble_init_callback(on_ble_init_callback),
                                           on_connected(on_connected),
                                           on_disconnected(on_disconnected) {}


void CGap::run() {
	// 1. Register on events to process callback function.
	_ble.onEventsToProcess(makeFunctionPointer(this, &CGap::scheduleBLEEventsToProcess));
	// 2. Register GAP event handler
	_ble.gap().setEventHandler(this);
	// 3. Initialize the BLE interface by registering a callback function
	_ble.init(this, &CGap::onBLEInitCompleteHandler);

	std::cout << "Starting BLE Application with device name \"" << _device_name << '\"' << std::endl;
	// 4. Dispatch events forever from the main thread
	_event_queue.dispatch_forever();
	// never reaches this line!
}

void CGap::startAdvertising() {
	// TODO:: Implement this functions
	// create an ble::AdvertisingParameters object with the specified configuration
	
	// 1. Create advertisement data
	ble::AdvertisingParameters adv_params(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,  // Type of advertising
        ble::adv_interval_t(100),                        // Min advertising interval (ms)
        ble::adv_interval_t(200),                        // Max advertising interval (ms)
        true                                             // Allow scan responses
    );  

	_adv_data_builder.setName(_device_name.c_str());
    _adv_data_builder.setFlags();

    uint16_t nordic_id = 0x0059;
    uint8_t manufacturer_data[6];

	// 2. setup the parameters
	manufacturer_data[0] = (uint8_t)(nordic_id & 0xFF);
    manufacturer_data[1] = (uint8_t)((nordic_id >> 8) & 0xFF);

	// this is an example error print. You can come up with your own ways to do so.
	// Note that the ble_error_t type variable name is error
    auto adv_data = _adv_data_builder.getAdvertisingData();
    if (adv_data.size() <= ble::LEGACY_ADVERTISING_MAX_SIZE) {
        memcpy(_adv_data_buffer, adv_data.data(), adv_data.size());
    }
    ble_error_t error = _ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_params);
    std::cout << bleErrorToString(error, "GAP::setAdvertisingParameters()") << std::endl;
	if (error != 0U) {
		return;
    }
	// 3. set the advertisement payload
	error = _ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE, _adv_data_builder.getAdvertisingData());
	std::cout << bleErrorToString(error, "GAP::setAdvertisingPayload()") << std::endl;
	if (error != 0U) {
		return;
	}

	std::cout << bleErrorToString(error, "GAP::setAdvertisingPayload()") << std::endl;
	if (error != 0U) {
		return;
	}
	

	// 4. Start advertising
	error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	std::cout << bleErrorToString(error, "GAP::startAdvertising()") << std::endl;
	if (error != 0U) {
		return;
	}
	std::cout << bleErrorToString(error, "GAP::startAdvertising()") << std::endl;
	
	// 5. Start blinking the Application LED
	
	std::cout << "Device is advertising" << std::endl;
}

void CGap::onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
	// TODO:: Implement this functions
	// 1. if on_connected callback is not nullptr, call it
    if (on_connected != nullptr) {
        on_connected();
    }
	// 2. keep the LED on
    _event_queue.cancel(_led_event_id);
	_led.write(0);
	std::cout << "Device is connected" << std::endl;
}
void CGap::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
	// TODO:: Implement this functions
	// 1. if on_disconnected is not nullptr, call it
    if (on_disconnected != nullptr) {
        on_disconnected();
    }
	// 2. start advertising
	_led_event_id = _event_queue.call_every(500ms, this, &CGap::toggleLED);
	std::cout << "Device is disconnected" << std::endl;
    startAdvertising();
}

void CGap::setOnBLEInitCompleteCallback(const mbed::Callback<void(ble::BLE &)> &callback) {
	_on_ble_init_callback = callback;	
}

void CGap::setOnConnectedCallback(const mbed::Callback<void(void)> &callback) {
    on_connected = callback;
}

void CGap::setOnDisconnectedCallback(const mbed::Callback<void(void)> &callback) {
    on_disconnected = callback;
}