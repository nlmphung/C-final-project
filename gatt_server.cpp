#include "gatt_server.h"

#include <iostream>

#include "ble_utils.h"

CGattServer::CGattServer() {
	// TODO:: Implement this functions
	// 1. initially started must be false
	_started = false;
    _services = {};
}

CGattServer::~CGattServer() {
	// TODO:: Implement this functions

	// Note that explicitly calling std::list::clear function destroys all of its elements.
	// This ensures that the services are destroyed properly since
	// their reference counts decrements to 1 (if the application is designed properly).
    BLE &ble = BLE::Instance();
    ble.gattServer().reset(); // idk if this is neccessary
    _services.clear();
	
}

bool CGattServer::addService(CGattService& service) {
	// TODO:: Implement this functions
	// 1. Check whether the server has started. If it has been started, we do not want to
	//    add new services when the server is running since it requires registering them with the
	//    system's GattServer instance
	if (_started) {
        return false;
    }
	// 2. Check whether the service is created by using its bool() operator
	//    i. if not, return false since we do not want to add not created services
	//    ii. if it is, push it to the _services list and return true
	if (!service) {
        return false;
    }
    _services.push_back(&service);
    // was false by default
	return true;
}

void CGattServer::startServer(ble::BLE& ble) {
	// TODO:: Implement this functions
	// 1. Register the services with the system's GattServer instance
	//    i. Iterate over each service
	//    ii. Add each service to the server using CGattService::registerService
	 GattServer& server = ble.gattServer();

    for (auto i : _services) {
        //server.addService(i); ??
        i->registerService(ble);
    }

	// 2. Register this object as the system's GattServer handler using GattServer::setEventHandler function
	server.setEventHandler(this);
}

void CGattServer::onConnected(void) {
	// TODO: implement this function
	// 1. Iterate over the _services
	// 2. call CGattService::onConnected function
	for (auto i: _services) {
        i->onConnected();
    }
}

void CGattServer::onDisconnected(void) {
	// TODO: implement this function
	// 1. Iterate over the _services
	// 2. call CGattService::onDisconnected function
	for (auto i : _services) {
        i->onDisconnected();
    }
}

void CGattServer::onDataWritten(const GattWriteCallbackParams& e) {
	// TODO:: Implement this functions
	GattCharacteristic* characteristic = nullptr;
	const char* characteristic_description = nullptr;
	const GattAttribute::Handle_t& value_handle = e.handle;
	std::cout << "onDataWritten() using Conn. Handle " << e.connHandle << " for Att. Handle " << value_handle
			  << std::endl;
	std::cout << "\twrite operation: " << e.writeOp << std::endl;
	std::cout << "\toffset: " << e.offset << std::endl;
	std::cout << "\tlength: " << e.len << std::endl;
	std::cout << "\tdata: ";
	std::cout << bufferToHex(e.data, e.len) << std::endl;
	// 1. Try to find the characteristic within the services that has the value_handle
	//    i. Iterate over the _services
    for (auto i : _services) {
        characteristic = i->getCharacteristicWithValueHandle(value_handle);
        if (characteristic == nullptr) {
            continue;
        }
        auto user_desc = i->getCharacteristicUserDescription(value_handle);
        if (user_desc == nullptr) {
            continue;
        }

        std::cout << "Characteristic with value handle \"" << user_desc << "\"" << std::endl;

        i->onDataWrittenHandler(characteristic, e.data, e.len);
        break;
    }
	//    ii. call CGattService::getCharacteristicWithValueHandle
	//    iii. if the calls returns a pointer different than nullptr
	//         a. Try to get user description of the characteristic by calling
	//         CGattService::getCharacteristicUserDescription function
	//            - if the call returns a valid address (different than nullptr), print it as follows
	//              `Characteristic with value handle "<user description str>"`
	//              Note surrounding " character of the name (exclude ` characters)
	//            - Otherwise, do not print anything
	//         b. Call CGattService::onDataWrittenHandler function with the required parameters
	//         c. Stop iterating over services
	//    iv. otherwise, continue iterating
	
	if (characteristic == nullptr) {
		std::cout << "\tThe characteristic cannot be found" << std::endl;
	}
}

void CGattServer::onDataRead(const GattReadCallbackParams& e) {
	// TODO:: Implement this functions
	GattCharacteristic* characteristic = nullptr;
	const char* characteristic_description = nullptr;
	const GattAttribute::Handle_t& value_handle = e.handle;
	std::cout << "onDataRead() using Conn. Handle " << e.connHandle << " for Att. Handle " << value_handle
			  << std::endl;
	// 1. Try to find the characteristic within the services that has the value_handle
	//    i. Iterate over the _services
	//    ii. call CGattService::getCharacteristicWithValueHandle
	//    iii. stop iterating when the calls returns a pointer different than nullptr
	//         after getting its description
	//         a. Try to get user description of the characteristic by calling
	//         CGattService::getCharacteristicUserDescription function
	//            - if the call returns a valid address (different than nullptr), print it as follows
	//              `Characteristic with value handle "<user description str>"`
	//              Note surrounding " character of the name (exclude ` characters)
	//            - Otherwise, do not print anything
	for (auto i : _services) {
        characteristic = i->getCharacteristicWithValueHandle(value_handle);
        if (characteristic == nullptr) {
            continue;
        }
        characteristic_description = i->getCharacteristicUserDescription(value_handle);
        if (characteristic_description == nullptr) {
            continue;
        }
        std::cout << "Characteristic with value handle \"" << characteristic_description << "\"" << std::endl;
        break;
    }
	if (characteristic == nullptr) {
		std::cout << "\tThe characteristic cannot be found" << std::endl;
	}
}

void CGattServer::onUpdatesEnabled(const GattUpdatesEnabledCallbackParams& params) {
	// TODO:: Implement this functions
	GattCharacteristic* characteristic = nullptr;
	const char* characteristic_description = nullptr;
	const GattAttribute::Handle_t& value_handle = params.charHandle;
	std::cout << "onUpdatesEnabled() using Conn. Handle " << params.connHandle << " for Att. Handle "
			  << value_handle << std::endl;
	// 1. Try to find the characteristic within the services that has the value_handle
	//    i. Iterate over the _services
	//    ii. call CGattService::getCharacteristicWithValueHandle
	//    iii. stop iterating when the calls returns a pointer different than nullptr
	//         after getting its description
	//         a. Try to get user description of the characteristic by calling
	//         CGattService::getCharacteristicUserDescription function
	//            - if the call returns a valid address (different than nullptr), print it as follows
	//              `Characteristic with value handle "<user description str>"`
	//              Note surrounding " character of the name (exclude ` characters)
	//            - Otherwise, do not print anything
	for (auto i : _services) {
        characteristic = i->getCharacteristicWithValueHandle(value_handle);
        if (characteristic == nullptr) {
            continue;
        }
        characteristic_description = i->getCharacteristicUserDescription(value_handle);
        if (characteristic_description == nullptr) {
            continue;
        }
        std::cout << "Characteristic with value handle \"" << characteristic_description << "\"" << std::endl;
        break;
    }
	if (characteristic == nullptr) {
		std::cout << "\tThe characteristic cannot be found" << std::endl;
	}
}

void CGattServer::onUpdatesDisabled(const GattUpdatesDisabledCallbackParams& params) {
	// TODO:: Implement this functions
	GattCharacteristic* characteristic = nullptr;
	const char* characteristic_description = nullptr;
	const GattAttribute::Handle_t& value_handle = params.charHandle;
	std::cout << "onUpdatesDisabled() using Conn. Handle " << params.connHandle << " for Att. Handle "
			  << value_handle << std::endl;
	// 1. Try to find the characteristic within the services that has the value_handle
	//    i. Iterate over the _services
	//    ii. call CGattService::getCharacteristicWithValueHandle
	//    iii. stop iterating when the calls returns a pointer different than nullptr
	//         after getting its description
	//         a. Try to get user description of the characteristic by calling
	//         CGattService::getCharacteristicUserDescription function
	//            - if the call returns a valid address (different than nullptr), print it as follows
	//              `Characteristic with value handle "<user description str>"`
	//              Note surrounding " character of the name (exclude ` characters)
	//            - Otherwise, do not print anything
	for (auto i : _services) {
        characteristic = i->getCharacteristicWithValueHandle(value_handle);
        if (characteristic == nullptr) {
            continue;
        }
        characteristic_description = i->getCharacteristicUserDescription(value_handle);
        if (characteristic_description == nullptr) {
            continue;
        }
        std::cout << "Characteristic with value handle \"" << characteristic_description << "\"" << std::endl;
        break;
    }
	if (characteristic == nullptr) {
		std::cout << "\tThe characteristic cannot be found" << std::endl;
	}
}

void CGattServer::onConfirmationReceived(const GattConfirmationReceivedCallbackParams& params) {
	// TODO:: Implement this functions
	GattCharacteristic* characteristic = nullptr;
	const char* characteristic_description = nullptr;
	const GattAttribute::Handle_t& value_handle = params.attHandle;
	std::cout << "onUpdatesDisabled() using Conn. Handle " << params.connHandle << " for Att. Handle "
			  << params.attHandle << std::endl;
	// 1. Try to find the characteristic within the services that has the value_handle
	//    i. Iterate over the _services
	//    ii. call CGattService::getCharacteristicWithValueHandle
	//    iii. stop iterating when the calls returns a pointer different than nullptr
	//         after getting its description
	//         a. Try to get user description of the characteristic by calling
	//         CGattService::getCharacteristicUserDescription function
	//            - if the call returns a valid address (different than nullptr), print it as follows
	//              `Characteristic with value handle "<user description str>"`
	//              Note surrounding " character of the name (exclude ` characters)
	//            - Otherwise, do not print anything
	for (auto i : _services) {
        characteristic = i->getCharacteristicWithValueHandle(value_handle);
        if (characteristic == nullptr) {
            continue;
        }
        characteristic_description = i->getCharacteristicUserDescription(value_handle);
        if (characteristic_description == nullptr) {
            continue;
        }
        std::cout << "Characteristic with value handle \"" << characteristic_description << "\"" << std::endl;
        break;
    }
	if (characteristic == nullptr) {
		std::cout << "\tThe characteristic cannot be found" << std::endl;
	}
}
