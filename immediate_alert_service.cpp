#include "immediate_alert_service.h"
#include "ble_utils.h"
#include <iostream>

const char* description = "ImmediateAlertService";

CImmediateAlertService::CImmediateAlertService(PinName led_pin)
	: _led(led_pin), _alert_level(IAS_ALERT_LEVEL_NO_ALERT), _alert_level_value(IAS_ALERT_LEVEL_NO_ALERT) {
	// TODO:: implement this constructor
	// 1. set the PWM period to 1 seconds
    _led.period(1.0);
    _led.write(1.0);
	// 2. create the characteristics with UUID GattCharacteristic::UUID_ALERT_LEVEL_CHAR
	//    assert that the characteristic is successfully created
    bool char_added = this->addCharacteristic(GattCharacteristic::UUID_ALERT_LEVEL_CHAR,
                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE,
                                              description,
                                              &_alert_level_value,
                                              sizeof(_alert_level_value)
                                              );
    assert(char_added);
    _characteristics[0]->setReadSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);
    _characteristics[0]->setWriteSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);
    _characteristics[0]->setUpdateSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);

	// 3. create the service with UUID GattService::UUID_IMMEDIATE_ALERT_SERVICE
	//    assert that the service is successfully created
    ble_error_t service_success = CGattService::createService(GattService::UUID_IMMEDIATE_ALERT_SERVICE);
    assert(service_success == BLE_ERROR_NONE);
    // NOTE::: You are required to provide UserDescription. If you do not, you might face HardFaults
    // since the template assumes the number of user description attributes is equal to the 
    // number characteristics
}

CImmediateAlertService::~CImmediateAlertService() {}

void CImmediateAlertService::onDataWrittenHandler(GattCharacteristic* characteristic,
												  const uint8_t* data,
												  uint16_t size) {
	// 1. Validate that characteristics has the value handle of this service's characteristic's value handle
    bool exists = this->getCharacteristicWithValueHandle(characteristic->getValueHandle());
    assert(exists);
	// 2. Cast the new value to AlertLevel enum type
    std::cout << "onDataWrittenHandler for immediate_alert_service data value: " << data << std::endl;
    AlertLevel level = (AlertLevel) (*data);
	// 3. Switch between supported different options
    std::cout << level << " < Level" << std::endl;
    switch (level) {
        case IAS_ALERT_LEVEL_NO_ALERT:
	//    i. NO_ALERT -> 100% duty cycle, off
            _led.write(1.0);
            break;
    //    ii. MEDIUM -> 75% duty cycle, 25% brightness
        case IAS_ALERT_LEVEL_MEDIUM:
            _led.write(0.75);
            break;
        case IAS_ALERT_LEVEL_HIGH:
    //    iii. HIGH  -> 10% duty cycle, 90% brightness 
            _led.write(0.1);
            break;

    }	
}

void CImmediateAlertService::onConnected(void) {
	_alert_level = IAS_ALERT_LEVEL_NO_ALERT;
	_alert_level_value = IAS_ALERT_LEVEL_NO_ALERT;
    GattCharacteristic& characteristic = *this->_characteristics[0];
	ble_error_t resp = this->setCharacteristicValue(characteristic, _alert_level_value);
    std::cout << "onDisconnected response: " << bleErrorToString(resp) << " " << std::endl; 	
}

void CImmediateAlertService::onDisconnected(void) {
	_alert_level = IAS_ALERT_LEVEL_MEDIUM;
	_alert_level_value = IAS_ALERT_LEVEL_MEDIUM;
	// set the characteristic value
    // get characteristic
    GattCharacteristic& characteristic = *this->_characteristics[0];
	ble_error_t resp = this->setCharacteristicValue(characteristic, _alert_level_value);
    std::cout << "onDisconnected response: " << bleErrorToString(resp) << " " << std::endl; 
}

void CImmediateAlertService::registerService(ble::BLE& ble) {
	// call the registerService function of the base class
	CGattService::registerService(ble);
}
