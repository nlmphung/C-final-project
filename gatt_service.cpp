#include "gatt_service.h"

#include <memory>
#include <iostream>

CGattService::CGattService() {}

CGattService::~CGattService() {
	// TODO:: Implement this function

	// 1. First release the service
	// you can use std::unique_ptr::reset function
    _service.reset();
	

	// 2. Destruct the characteristics
	for (auto i : _characteristics) {
        delete i;
    }

	// 3. Destruct the characteristic descriptors
	for (auto i : _characteristics_user_descriptions) {
        for (auto j : i) {
            delete j;
        }
    }
}

bool CGattService::addCharacteristic(
	const UUID &uuid,
	const uint8_t properties /*= GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NONE*/,
	const char *user_description /*= nullptr*/,
	uint8_t *value /*= nullptr*/,
	size_t max_value_size /*= 0*/) {
	// TODO:: Implement this function

	// 1. check whether the service has not been created yet.
	if (_service) {
		// if it is, we cannot add characteristics to it.
		return false;
	}
	// 2. create the user description attribute
	//  i. create a vector for the characteristic descriptors. You might want to check
	//      std::vector::emplace_back and std::vector::back functions.
    _characteristics_user_descriptions.emplace_back();
    auto &descriptors = _characteristics_user_descriptions.back();
	//  ii. if the user_description is not empty
	//      a. create (by dynamically allocating) a new attribute. assert that the attribute memory can be
	//          allocated. // NOTE:: You cannot use static_assert function. Why?
	//      b. add it to the just created list
    if (user_description != nullptr) {
        // unique pointer here? 
        GattAttribute *attribute = new GattAttribute(BLE_UUID_DESCRIPTOR_CHAR_USER_DESC,/*Bluetooth defined UUID for CHARACTERISTIC_DESCRIPTOR*/
                            (uint8_t *)user_description, /*Value of this attribute*/
                            sizeof(user_description), /*size of the initial value*/
                            sizeof(user_description), /*maximum size the attribute value might have*/
                            false /*this attribute has fixed size */);
        assert(attribute != nullptr);

        descriptors.emplace_back(attribute);    
    }
	//  iii. If the user_description is empty, just keep the list empty
	
	// 3. create the characteristic by using the fact that std::vector places its element
	//    in contiguous memory region to get the characteristic attributes
	//
	//    allocate new entry to service
	//    i. allocate a new characteristic. Note that len is equal max_value_size when value is not nullptr.
	//       maxLen is always equal to max_value_size
    GattCharacteristic* characteristic = new GattCharacteristic(UUID(uuid), /*UUID of the characteristic*/
                                  value, /*Memory buffer holding the initial value. */
                                  (value != nullptr) ? max_value_size: 0, /*The size in bytes of the characteristic's value*/
                                  max_value_size, /*The capacity in bytes of the characteristic value buffer*/
                                  properties, /*An 8-bit field that contains the characteristic's properties*/
                                  descriptors.empty() ? nullptr : &descriptors[0], /*A pointer to an array of descriptors to be included within this characteristic.*/
                                  descriptors.size(), /*The number of descriptors presents in descriptors array*/
                                  false /*Flag that indicates if the attribute's value has variable length*/);
	//    ii. assert that the attribute memory can be
	//          allocated. // NOTE:: You cannot use static_assert function. Why?
    assert(characteristic != nullptr);
	//    iii. add the characteristic to _characteristics
	_characteristics.emplace_back(characteristic);
	return true;
	
}

GattCharacteristic *CGattService::getCharacteristicWithValueHandle(
	const GattAttribute::Handle_t &value_handle) const {
	// TODO:: Implement this function
	// 1. Iterate over _characteristics
	//    i. For each characteristic, compare its value handle with the argument value handle
	//    ii. If a characteristic has a value handle, return it.
	//    iii. If no characteristic has the indicated value handle, return nullptr.
	for (auto i : _characteristics) {
        if (i->getValueHandle() == value_handle ) { // Handle_t type should be an Id for the value
            return i;
        }
    }
	return nullptr;
}

const char *CGattService::getCharacteristicUserDescription(
	const GattAttribute::Handle_t &value_handle) const {
	// TODO:: Implement this function
	int characteristic_index = 0;
	// 1. Iterate over _characteristics
	//    i. For each characteristic, compare its value handle with the argument value handle
	//    ii. If a characteristic has the value handle value_handle,
	//        a. Get the descriptors of that characteristic in _characteristics_user_descriptions vector
	//        b. Get the first descriptor
	//        c. Get the value pointer of the attribute using you can use GattAttribute::getValuePtr function.
	//        d. Cast it to const char*
	//    iii. Otherwise, increase characteristic index
	//    iv. If no characteristic has the indicated value handle, return nullptr.
	int index = 0;
    for (auto i : _characteristics) {
        if (i->getValueHandle() == value_handle) {
            // _characteristics_user_descriptions - GattAttributes there. So the vector of Attributes
            // assuming these are in the same order
            auto &descriptions = _characteristics_user_descriptions.at(index);
            GattAttribute* user_description = descriptions.at(0);
            const char* user_description_pointer = (const char*) (user_description->getValuePtr());
            return user_description_pointer;
        }
        index++;
    }

	return nullptr;
}

ble_error_t CGattService::createService(const UUID &uuid) {
	// TODO:: Implement this function

	// 1. assert that the service has already been created.
	//    if it has been created, and this function is called again,
	//    there is something wrong with the software design
	assert(!_service && "createService was called twice, some error");
	// 2. check whether the service has some characteristics that can be added to the service
	//    If there is no characteristics, return BLE_ERROR_INVALID_STATE
	if (_characteristics.empty()) {
        return BLE_ERROR_INVALID_STATE;
    }

	// 3. create a shared pointer for the service
	//    you can use std::make_unique function
	_service = std::make_unique<GattService>(GattService(uuid,
                                        &_characteristics[0],
                                        _characteristics.size()));
	// 4. assert that the _service memory has been successfully allocated
	assert(_service != nullptr && "Service pointer allocation failed");
	// 5. Return BLE_ERROR_NONE
	return BLE_ERROR_NONE;
	
}

GattService *CGattService::getService() const {
	// TODO:: Implement this function
	return _service.get();
	
}

unsigned int CGattService::getCharacteristicCount() const {
	// TODO:: Implement this function
	return static_cast<unsigned int>(_characteristics.size());
	
}

void CGattService::registerService(ble::BLE &ble) {
	// TODO:: Implement this function
	ble.gattServer().addService(*_service);
}