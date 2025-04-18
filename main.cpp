#include <BLE.h>
#include <mbed.h>

#include <iostream>

#include "alert_notification_service.h"
#include "gatt_server.h"
#include "immediate_alert_service.h"
#include "nrf52832_app_protect.h"
#include "secure_gap.h"

int main() {
	/////////////////////////////////////////////////////////
	// the following must always present in main files
	nrf52_disable_approtect();
	///////////////////////////////////////////////////////////
	/*---------------------------------------------------------*/

	// TODO:: Implement this functions

	std::cout << "Starting GattServer application" << std::endl;

	

	// 1. Create EventQueue object for the application
    EventQueue event_queue;
	// 2. Get a reference to system's BLE instance
    BLE& ble = BLE::Instance();
	// 3. Instantiate CSecureGap object with the required parameters
    CSecureGap csecuregap(ble, event_queue, "MTC-BLE", LED1);
	// 4. Instantiate a CImmediateAlertService object and a CAlertNotificationService object
    CImmediateAlertService calert(LED2);
    CAlertNotificationService cnotif(event_queue, BUTTON1);
	// 5. instantiate CGattServer object
    CGattServer cgatt;
	// 6. Add service objects to CGattServer
    cgatt.addService(calert);
    cgatt.addService(cnotif);
	// 7. Set onBLEInitComplete callback of Gap to CGattServer::startServer function
	//    of the instantiated object.
    csecuregap.setOnBLEInitCompleteCallback(callback(&cgatt, &CGattServer::startServer));
	// 8. Set onConnected callback of Gap to CGattServer::onConnected function
	//    of the instantiated object.
    csecuregap.setOnConnectedCallback(callback(&cgatt, &CGattServer::onConnected));
	// 9. Set onDisconnected callback of Gap to CGattServer::onDisconnected function
	//    of the instantiated object.
    csecuregap.setOnDisconnectedCallback(callback(&cgatt, &CGattServer::onDisconnected));
	// 10. Call CSecureGap::run member function to start the application
    csecuregap.run();
	return 0;
}