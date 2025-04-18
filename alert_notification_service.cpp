#include "alert_notification_service.h"

#include <iostream>

#include "ble_utils.h"


void CAlertNotificationService::buttonPressedISR(){
    // dispatch the Button event handling to OS context using Application's EventQueue object.
    _event_queue.call(callback(this, &CAlertNotificationService::buttonPressedHandler));
}

void CAlertNotificationService::buttonPressedHandler() {
    _button_press_count++;
    addNewAlertToCategory(CategoryId::ANS_TYPE_SIMPLE_ALERT);
    std::cout << "Button pressed - added new simple alert. Button press count: " << (int)_button_press_count << std::endl;
}

const UUID uuids_array[5] = {GattCharacteristic::UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR, GattCharacteristic::UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR, 
                           GattCharacteristic::UUID_UNREAD_ALERT_CHAR, GattCharacteristic::UUID_NEW_ALERT_CHAR, 
                           GattCharacteristic::UUID_ALERT_NOTIFICATION_CONTROL_POINT_CHAR};
enum GattCharacteristic::Properties_t properties_array[5] = {GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY,
                                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY,
                                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE};
                                                            
const char* char_descriptions[5] = {"Supported New Alert Category", "Supported Unread Alert Category",
                                       "Unread Alert", "New Alert", "Alert Notification Control Point"};

uint8_t categoryMask = (uint8_t)CAlertNotificationService::getCategoryMaskFromId(CAlertNotificationService::CategoryId::ANS_TYPE_SIMPLE_ALERT);


CAlertNotificationService::CAlertNotificationService(EventQueue &event_queue, PinName button_pin) : _button_press_count(0), 
_button(button_pin),
_event_queue(event_queue) {
	// TODO:: implement this constructor
	// 1. initialize the class attributes to 0
	//      1. _supported_new_alert_category
	//      2. _supported_unread_alert_category
	//      3. _enabled_new_alert_category
	//      4. _enabled_unread_alert_category
    //      NOTE::: Do not forget to set UserDescription for each characteristic. This template assumes that each Characteristic
    //      has a user description string. If you do not want to use user description, you can set the string to empty string, 
    //      but not to nullptr.
    _supported_new_alert_category = 0;
    _supported_unread_alert_category = 0;
    _enabled_new_alert_category = 0;
    _enabled_unread_alert_category = 0;
    // 2. _alert_status -- for each category, set the alert counts to 0 but correctly initialize the
	//    category field
    _supported_new_alert_category_index = 0;
    _supported_unread_alert_category_index = 1;
    _unread_alert_status_index = 2;
    _new_alert_index = 3;
    _alert_notification_control_point_index = 4;

    _control_point.fields.command = ANS_DISABLE_NEW_INCOMING_ALERT_NOTIFICATION;
    _control_point.fields.category = ANS_TYPE_ALL_ALERTS;
    

    _connected = false;
    // Initialize the categories and alert statuses
    for (int i = 0; i < ANS_ALERT_CATEGORY_COUNT; i++) {
        _alert_status[i].fields.category = i;
        _alert_status[i].fields.count = 0;
    }

	// 2. create the characteristics with UUIDs declared under GattService class namespace
	//      GattCharacteristic::UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR
	//      GattCharacteristic::UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR
	//      GattCharacteristic::UUID_UNREAD_ALERT_CHAR
	//      GattCharacteristic::UUID_NEW_ALERT_CHAR
	//      GattCharacteristic::UUID_ALERT_NOTIFICATION_CONTROL_POINT_CHAR
    uint8_t newCategoryMask = (uint8_t)_supported_new_alert_category;
    uint8_t unreadCategoryMask = (uint8_t)_supported_unread_alert_category;
    for (size_t i = 0; i < 5; ++i) {
        bool success = this->addCharacteristic(uuids_array[i],
                                properties_array[i],
                                char_descriptions[i],
                                (i == 0) ? &newCategoryMask :
                                (i == 1) ? &unreadCategoryMask :
                                (i == 2) ? &_alert_status[i].fields.count :
                                (i == 3) ? &_alert_status[i].fields.count :
                                (i == 4) ? 0: nullptr,
                                2);
        assert(success);
        _characteristics[i]->setReadSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);
        _characteristics[i]->setWriteSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);
        _characteristics[i]->setUpdateSecurityRequirement(ble::att_security_requirement_t::AUTHENTICATED);
    }

    //COMMENT: The two lines below are redundant as thay are handled 
    /* in addSupportedNewAlertsCategory and AddSupportedUnreadAlertCategory.

    FIX: We need to call them to set the support for those categories. The commands only set
    enabled flags. The supported flags need to be set by the server (aka this constructor).
		*/
    setSupportedNewAlertsCategory(ANS_TYPE_MASK_SIMPLE_ALERT);
    setSupportedUnreadAlertsCategory(ANS_TYPE_MASK_SIMPLE_ALERT);


	//    As you did in Modules 8, follow the characteristics indices as the creation order
	
	// 3. create the service with UUID GattService::UUID_ALERT_NOTIFICATION_SERVICE
	ble_error_t error = CGattService::createService(GattService::UUID_ALERT_NOTIFICATION_SERVICE);
    std::cout << "Alert notification service creation error: " << bleErrorToString(error) << std::endl;
    assert(error == BLE_ERROR_NONE);
	// 4. configure Falling edge ISR function for the button
	_button.fall(callback(this, &CAlertNotificationService::buttonPressedISR));
}


CAlertNotificationService::~CAlertNotificationService() {}

void CAlertNotificationService::onDataWrittenHandler(GattCharacteristic* characteristic,
													 const uint8_t* data,
													 uint16_t size) {

    int characteristic_index = -1;
    size_t index = 0;

    for (std::vector<GattCharacteristic*>::iterator it = _characteristics.begin(); it != _characteristics.end(); ++it, ++index) {
        if (*it == characteristic) {
            characteristic_index = index;
            break;
        }
    }

    if (characteristic_index == -1) {
        return;
    }

	// You are required check whether the characteristic is the Alert Notification Control Point
    for (size_t i = 0; i < _characteristics.size(); ++i) {
        if (characteristic_index == _alert_notification_control_point_index) {
                if (size < 1 || size > 2) {
                    std::cout << "Size < 1 or size > 2, not processing command." << std::endl;
                    return;
                }

                if (size == 1) {
                    for (int j = 0; j < ANS_ALERT_CATEGORY_COUNT; ++j) {
                        _alert_status[j].fields.count = 0;
                    }
                    std::cout << "Size == 1, setting number of alerts for all categories as 0" << std::endl;

                } else if (size == 2) {
                    // COMMENT: 1 and 0 seem to be inverted in two lines below
					// FIX: flipped them
                    uint8_t category = data[1]; // CategoryId
                    uint8_t command = data[0];  // CommandId
                    std::cout << "COMMAND FOR CATEGORY " << (int)category << ". COMMAND VALUE: " << (int)command << std::endl;

                    _alert_status[category].fields.category = category;
                    _control_point.fields.command = command;
                    _control_point.fields.category = category;
					uint16_t categoryMask = getCategoryMaskFromId((CAlertNotificationService::CategoryId)_control_point.fields.category);

                    switch (command) {
                        case ANS_ENABLE_NEW_INCOMING_ALERT_NOTIFICATION:
                            std::cout << "Trying to enable new alert notifications for category with category value: " << 
                              (int)category << std::endl;
                              // COMMENT: all of these bitwise operations should use
                              // The category mask, not the category ID
                              // For example the below should be 
                              // int categoryMask = getCategoryMaskFromId((CAlertNotificationService::CategoryId)controlPoint.fields.category);
                              //_enabled_new_alert_category |= categoryMask;
                              // (you should probably define the category mask above the switch)
                              // FIX: did that, except used uint16_t instead of int
                              if (_supported_new_alert_category & categoryMask) {
                                    _enabled_new_alert_category |= categoryMask;
                                    std::cout << "Supported and enabled." << std::endl;
                              } else {
                                  std::cout << "Category not supported." << std::endl;
                              }
                            break;
                        case ANS_ENABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION:
                            std::cout << "Trying to enable unread alert notifications for category with category value " <<
                              (int)category << std::endl;                            
                            if (_supported_unread_alert_category & categoryMask) {
                                _enabled_unread_alert_category |= categoryMask;
                                std::cout << "Supported and enabled." << std::endl;
                              } else {
                                  std::cout << "Category not supported." << std::endl;
                              }
                            break;
                        case ANS_DISABLE_NEW_INCOMING_ALERT_NOTIFICATION:
                                std::cout << "Disabled new alert notifications for category with category value: " << 
                              (int)category << std::endl;

                            std::cout << "Trying to disable new alert notifications for category " << (int)category << std::endl;
                              if (_supported_new_alert_category & categoryMask) {
                                _enabled_new_alert_category &= ~categoryMask;
                                std::cout << "Supported and now disabled." << std::endl;
                              } else {
                                  std::cout << "Category not supported." << std::endl;
                              }
                            
                            // COMMENT: no need to invert category, just do the following:  _enabled_new_alert_category &= categoryMask;
                            // FIX: We are removing the category so a bitwise & with 0 is what we want
                            // Inverting the mask sets all other bits to 1 so & retains their values but unsets the bit of the category
                            // we want to disable.
                            break;
                        case ANS_DISABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION:
                            std::cout << "Trying to disable unread alert notifications for category with category value: " << 
                              (int)category << std::endl;
                              if (_supported_unread_alert_category & categoryMask) {
                                _enabled_unread_alert_category &= ~categoryMask;
                                std::cout << "Supported and now disabled." << std::endl;
                              } else {
                                  std::cout << "Category not supported." << std::endl;
                              }

                            //COMMENT: same as above
                            break;
                        // So what are these really - implemented as in the spesification or something else?
                        //COMMENT: these are service behaviours as defined in pages 14 to 16 of the spec
						// 
						// From spesification: Notify the New Alert characteristic to the client immediately for the category specified in the Category ID field if that
                        // category is enabled. If there are no new alerts for specified category ID on the server, the value for the “Number of New Alert”
                        // field shall be set to 0. If the category ID is specified as 0xff, the New Alert characteristics for all currently enabled categories shall be notified.
                        case ANS_NOTIFY_NEW_INCOMING_ALERT_IMMEDIATELY:
														if (category == 0xff) {
															// COMMENT: handle category == ANS_TYPE_ALL_ALERTS == 0xFF with a for loop notifying all
                                                            // FIX: done
                                                            std::cout << "Notifying all categories of new incoming alert." << std::endl;
															 for (int i = 0; i <= ANS_ALERT_CATEGORY_COUNT; i++) {
                                                                 uint16_t mask = 1 << i;
																 if (_enabled_new_alert_category & mask) {
																	CGattService::setCharacteristicValue(*_characteristics[_new_alert_index], _button_press_count);
																}
															 }
														} else if (_enabled_new_alert_category & categoryMask) {
                                                            std::cout << "Notify characteristic with mask value " << (int)categoryMask << " and category value " << (int)category << " immediately" << std::endl;
                                                            CGattService::setCharacteristicValue(*_characteristics[_new_alert_index], _alert_status[category].fields.count);
                                                        } else {
                                                            std::cout << "Tried to notify of a new incoming alert with a command. The spesified category is not enabled." << std::endl;                                                           
                                                        }
                            break;
                        case ANS_NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY:
                            std::cout << "Trying to notify unread alert category with id value: " << 
                              (int)category << " immediately" << std::endl;
                            if (_enabled_unread_alert_category & categoryMask) {
                                CGattService::setCharacteristicValue(*_characteristics[_unread_alert_status_index], _alert_status[category].fields.count);
                            } else {
                                std::cout << "Category not enabled." << std::endl;
                            }
                            break;

                        default:
                        break;
                    }
                }
            break;
            }
    }
	// 1. if it is, check the size of the data
	//      i. if the size is less than 1, return
	//      ii. if the size is larger than 2, return
	// 4. if the size is equal to 1, set the alert status to zero
	// 5. if the size is equal to 2, set the alert status to zero and set the category field
	//    of the alert status to the value of data[0]
	// 6. Handle the commands as described in the specification.
	// HINT:: Consider printing messages for each command received.
	// bleErrorToString function can be used to print the error codes.
	
}


void CAlertNotificationService::onConnected(void) {
	// 1. clear all enabled alerts
    clearAlertsOfCategory(CategoryId::ANS_TYPE_ALL_ALERTS);
    _button_press_count = 0;
	// 2. call the clearAlertsOfCategory function with all categories
	// 3. set the _connected attribute to true
    _connected = true;
	
}

void CAlertNotificationService::onDisconnected(void) {
	// 1. set the _connected attribute to false
    _connected = false;
	// 2. call the clearAlertsOfCategory function with all categories
	clearAlertsOfCategory(CategoryId::ANS_TYPE_ALL_ALERTS);
}

void CAlertNotificationService::registerService(ble::BLE& ble) {
	// call the registerService function of the base class
	CGattService::registerService(ble);
}

bool CAlertNotificationService::addNewAlertToCategory(CAlertNotificationService::CategoryId category) {
	// 1. check whether the category is supported
    uint16_t category_mask = getCategoryMaskFromId(category);
	if (!(category_mask & _supported_new_alert_category)) {
        std::cout << "Category with id " << category << " not supported." << std::endl;
        return false; 
    }
	// 2. add new alert to the category
	_alert_status[category].fields.count++;
    std::cout << "Added alert to category " << category << ". Count: " << 
        (int)_alert_status[category].fields.count << std::endl;
	// 3. check whether the category is enabled `new_alert_category`
	//  ii. if it is not, return


    // TEAM NOTE: This means that if new alerts are disabled but new unread alerts are enabled the 
    // button presses do not notify of new unread alerts even if new unread alerts are enabled.


	uint16_t enabled = category_mask & _enabled_new_alert_category;
    if (!enabled) {
        std::cout << "Category with id " << category << " not enabled. Bitwise and with mask: " << enabled << std::endl;
        return false;
    }
    //   i. if it is, notify the new alert characteristic
    CGattService::setCharacteristicValue(*_characteristics[_new_alert_index], _alert_status[category].fields.count);


	// 4. check whether the category is enabled `unread_alert_category`
	//   i. if it is, notify the unread alert characteristic
	//  ii. if it is not, return
	uint16_t enabled_unread = category_mask & _enabled_unread_alert_category;
    if (!enabled_unread) {
        std::cout << "Category with id " << category << " unread alerts not enabled. Bitwise and with mask: " << enabled << std::endl;
        return false;
    }
    CGattService::setCharacteristicValue(*_characteristics[_unread_alert_status_index], _alert_status[category].fields.count);
	return true;
}

// COMMENT: The six funtions below need to notify the client with setCharacteristicValue()
// I have given replaced the two first ones with the model solution as an example. 
// Try to figure out the 4 below them
// FIX: implemented but only tested the set functions as they are the only ones we use.

bool CAlertNotificationService::setSupportedUnreadAlertsCategory(uint16_t supportedUnreadAlerts) {
	// TODO: implement this function
	// 1. check whether the peer is connected
	//      i. if it is, return false
	// 2. set the supported unread alerts category value
	// 3. clear the alerts of the supported unread alerts category
	if (_connected) {
		return false;
	}
	_supported_unread_alert_category = supportedUnreadAlerts;
	for (int ii = 0; ii < ANS_ALERT_CATEGORY_COUNT; ii++) {
		if ((supportedUnreadAlerts & (1 << ii)) != 0) {
			clearAlertsOfCategory((CAlertNotificationService::CategoryId)ii);
		}
	}
	auto retCode = setCharacteristicValue(*_characteristics[_supported_unread_alert_category_index],
										  _supported_unread_alert_category);

	std::cout << bleErrorToString(retCode, "Setting the supported unread alert characteristic\n\t") << std::endl;

	return true;
}

bool CAlertNotificationService::addSupportedUnreadAlertsCategory(CAlertNotificationService::CategoryId category) {
	// TODO: implement this function
	// 1. check whether the peer is connected
	//      i. if it is, return false
	// 2. update the supported unread alerts category value
	// 3. clear the alerts of the supported unread alerts category
	if (_connected) {
		return false;
	}
	_supported_unread_alert_category |= getCategoryMaskFromId(category);
	clearAlertsOfCategory(category);
	auto retCode = setCharacteristicValue(*_characteristics[_supported_unread_alert_category_index],
										  _supported_unread_alert_category);

	std::cout << "Adding supported unread alert characteristic\n\t" << bleErrorToString(retCode) << std::endl;

	return true;
}

bool CAlertNotificationService::removeSupportedUnreadAlertsCategory(CAlertNotificationService::CategoryId category) {
	// TODO: implement this function
	// 1. check whether the peer is connected
	//      i. if it is, return false
    if (_connected) {
        return false;
    }
	// 3. update the supported unread alerts category value
    uint16_t category_mask = getCategoryMaskFromId(category);
    _supported_unread_alert_category &= ~category_mask;
	// 4. clear the alerts of the supported unread alerts category
	// NOTE::: Make sure to handle ANS_TYPE_ALL_ALERTS category correctly!
    // TEAM NOTE: only removed alerts of the category to be removed
    auto retCode = setCharacteristicValue(*_characteristics[_supported_unread_alert_category_index],
										  _supported_unread_alert_category);

	std::cout << "Removing the supported unread alert characteristic\n\t" << bleErrorToString(retCode) << std::endl;

	clearAlertsOfCategory(category);

	return true;
	
}

bool CAlertNotificationService::setSupportedNewAlertsCategory(uint16_t supportedNewAlerts) {
	// TODO: implement this function
	// 1. check whether the peer is connected
	//      i. if it is, return false
    if (_connected) {
        return false;
    }
	// 2. set the supported new alerts category value
    _supported_new_alert_category = supportedNewAlerts;
	// 3. clear the alerts of the supported unread alerts category
	for (int ii = 0; ii < ANS_ALERT_CATEGORY_COUNT; ii++) {
		if ((supportedNewAlerts & (1 << ii)) != 0) {
			clearAlertsOfCategory((CAlertNotificationService::CategoryId)ii);
		}
	}
    auto retCode = setCharacteristicValue(*_characteristics[_supported_new_alert_category_index],
										  _supported_new_alert_category);

	std::cout << bleErrorToString(retCode, "Setting the supported new alert characteristic\n\t") << std::endl;
	
	return true;
	
}

bool CAlertNotificationService::addSupportedNewAlertsCategory(CAlertNotificationService::CategoryId category) {
	// TODO: implement this function
	// 1. check whether the peer is connected
	//      i. if it is, return false
	// 2. update the supported new alerts category value
	// 3. clear the alerts of the supported unread alerts category
    if (_connected) {
        return false;
    }
    uint16_t category_mask = getCategoryMaskFromId(category);
    _supported_new_alert_category |= category_mask;
    // TEAM NOTE: same as before
    auto retCode = setCharacteristicValue(*_characteristics[_supported_new_alert_category_index],
										  _supported_new_alert_category);

	std::cout << bleErrorToString(retCode, "Setting the supported unread alert characteristic\n\t") << std::endl;

	clearAlertsOfCategory(category);
	return true;
}

bool CAlertNotificationService::removeSupportedNewAlertsCategory(CAlertNotificationService::CategoryId category) {
   if (_connected) {
        return false;
    }
	// 3. update the supported unread alerts category value
    uint16_t category_mask = getCategoryMaskFromId(category);
    _supported_new_alert_category &= ~category_mask;
	// 4. clear the alerts of the supported unread alerts category
    auto retCode = setCharacteristicValue(*_characteristics[_supported_new_alert_category_index],
										  _supported_new_alert_category);

	std::cout << bleErrorToString(retCode, "Removing the supported unread alert characteristic\n\t") << std::endl;

	// NOTE::: Make sure to handle ANS_TYPE_ALL_ALERTS category correctly!
	clearAlertsOfCategory(category);
	return true;	
}

void CAlertNotificationService::clearAlertsOfCategory(CategoryId category) {
	// TODO: implement this function
	// 1. check whether the category is supported
	// 2. clear all alerts of the category
switch (category) {
    case ANS_TYPE_SIMPLE_ALERT:
        _alert_status[ANS_TYPE_SIMPLE_ALERT].fields.count = 0;
        break;
    case ANS_TYPE_EMAIL:
        _alert_status[ANS_TYPE_EMAIL].fields.count = 0;
        break;
    case ANS_TYPE_NEWS:
        _alert_status[ANS_TYPE_NEWS].fields.count = 0;
        break;
    case ANS_TYPE_NOTIFICATION_CALL:
        _alert_status[ANS_TYPE_NOTIFICATION_CALL].fields.count = 0;
        break;
    case ANS_TYPE_MISSED_CALL:
        _alert_status[ANS_TYPE_MISSED_CALL].fields.count = 0;
        break;
    case ANS_TYPE_SMS_MMS:
        _alert_status[ANS_TYPE_SMS_MMS].fields.count = 0;
        break;
    case ANS_TYPE_VOICE_MAIL:
        _alert_status[ANS_TYPE_VOICE_MAIL].fields.count = 0;
        break;
    case ANS_TYPE_SCHEDULE:
        _alert_status[ANS_TYPE_SCHEDULE].fields.count = 0;
        break;
    case ANS_TYPE_HIGH_PRIORITIZED_ALERT:
        _alert_status[ANS_TYPE_HIGH_PRIORITIZED_ALERT].fields.count = 0;
        break;
    case ANS_TYPE_INSTANT_MESSAGE:
        _alert_status[ANS_TYPE_INSTANT_MESSAGE].fields.count = 0;
        break;
    case ANS_TYPE_ALL_ALERTS:
        // Clear all categories
        for (int i = 0; i < ANS_ALERT_CATEGORY_COUNT; i++) {
            _alert_status[i].fields.count = 0;
        }
        break;
    default:
        // Handle unexpected category
        break;
    }
	
}
