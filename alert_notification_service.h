#pragma once
#include <BLE.h>
#include <mbed.h>

#include <map>
#include <memory>
#include <vector>

#include "gatt_service.h"

/**
 * @brief Implements the Alert Notification Service (ANS) as defined in the Bluetooth Specification.
 *
 * This service provides a mechanism for notifying a client device about new and unread alerts, categorized
 * by type (e.g., email, SMS, call). The service uses several characteristics to manage alert categories,
 * counts, and control notifications. See the Bluetooth SIG specification for details:
 * [https://www.bluetooth.com/specifications/specs/alert-notification-service-1-0/](https://www.bluetooth.com/specifications/specs/alert-notification-service-1-0/)
 *
 * @details The service manages alerts across multiple categories using a bitmask approach.
 * Each category (defined in `CategoryId` enum) is assigned a bit in the configuration bitfields.
 * The `addNewAlertToCategory` function increments the alert count for a specified category.
 * The service supports enabling and disabling notifications for new alerts and unread alert status.
 * These functions manage both the internal state and the notification to the peer device.
 *
 * The service utilizes the following characteristics:
 * - `GattCharacteristic::UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR`: Read-only characteristic indicating supported new
 * alert categories.
 * - `GattCharacteristic::UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR`: Read-only characteristic indicating supported
 * unread alert categories.
 * - `GattCharacterist<1ic::UUID_UNREAD_ALERT_CHAR`: Notification characteristic providing the count of unread alerts per
 * category.
 * - `GattCharacteristic::UUID_NEW_ALERT_CHAR`: Notification characteristic signaling new alerts in specific categories.
 * - `GattCharacteristic::UUID_ALERT_NOTIFICATION_CONTROL_POINT_CHAR`: Write-without-response characteristic for
 * controlling notification enabling/disabling.
 *
 * A button press triggers an alert using `buttonPressedHandler` which adds a new alert to the `ANS_TYPE_SIMPLE_ALERT`
 * category. Security requirements for each characteristic are defined in the constructor.
 */
class CAlertNotificationService : public CGattService {
   public:
#define ANS_ALERT_CATEGORY_COUNT (10)

	enum CategoryId {
		/**
		 * @brief Alert categories.
		 *
		 */
		ANS_TYPE_SIMPLE_ALERT = 0,			 /**< General text alert or non-text alert.*/
		ANS_TYPE_EMAIL = 1,					 /**< Email message arrives.*/
		ANS_TYPE_NEWS = 2,					 /**< News feeds such as RSS, Atom. */
		ANS_TYPE_NOTIFICATION_CALL = 3,		 /**< Incoming call. */
		ANS_TYPE_MISSED_CALL = 4,			 /**< Missed call. */
		ANS_TYPE_SMS_MMS = 5,				 /**< SMS or MMS message arrives. */
		ANS_TYPE_VOICE_MAIL = 6,			 /**< Voice mail. */
		ANS_TYPE_SCHEDULE = 7,				 /**< Alert that occurs on calendar, planner. */
		ANS_TYPE_HIGH_PRIORITIZED_ALERT = 8, /**< Alert to be handled as high priority. */
		ANS_TYPE_INSTANT_MESSAGE = 9,		 /**< Alert for incoming instant messages. */
		ANS_TYPE_ALL_ALERTS = 0xFF			 /**< Identifies all alerts. */
	};

	/**
	 * @brief Alert category mask IDs.
	 *
	 * These masks are used to represent which alert categories are supported or enabled.
	 * Each category is assigned a unique bit in the mask.
	 *
	 * @note The `ANS_TYPE_MASK_ALL_ALERTS` mask is used to represent all alert categories.
	 *       It is a bitwise OR of all other category masks.
	 */
	enum CategoryMaskId {
		ANS_TYPE_MASK_SIMPLE_ALERT = (1 << 0),			 /**< General text alert or non-text alert.*/
		ANS_TYPE_MASK_EMAIL = (1 << 1),					 /**< Email message arrives. */
		ANS_TYPE_MASK_NEWS = (1 << 2),					 /**< News feeds such as RSS, Atom.*/
		ANS_TYPE_MASK_NOTIFICATION_CALL = (1 << 3),		 /**< Incoming call.*/
		ANS_TYPE_MASK_MISSED_CALL = (1 << 4),			 /**< Missed call.*/
		ANS_TYPE_MASK_SMS_MMS = (1 << 5),				 /**< SMS or MMS message arrives.*/
		ANS_TYPE_MASK_VOICE_MAIL = (1 << 6),			 /**< Voice mail.*/
		ANS_TYPE_MASK_SCHEDULE = (1 << 7),				 /**< Alert that occurs on calendar, planner.*/
		ANS_TYPE_MASK_HIGH_PRIORITIZED_ALERT = (1 << 8), /**< Alert to be handled as high priority.*/
		ANS_TYPE_MASK_INSTANT_MESSAGE = (1 << 9),		 /**< Alert for incoming instant messages.*/
		ANS_TYPE_MASK_ALL_ALERTS = 0x03FF				 /**< Identifies all alerts. */
	};

	/**
	 * @brief Get the CategoryId from the mask
	 *
	 * @param mask The mask to be converted to CategoryId
	 * @return CategoryId The category id
	 */
	static CategoryId getCategoryIdFromMask(uint16_t mask) {
		int category = 0;
		for (int ii = 0; ii < ANS_ALERT_CATEGORY_COUNT; ii++) {
			if ((mask & (1 << ii)) != 0) {
				category = ii;
				break;
			}
		}
		return (CategoryId)category;
	}

	/**
	 * @brief Get the Category mask from the CategoryId
	 *
	 * @param category The category id to be converted to mask
	 * @return uint16_t The category mask
	 */
	static uint16_t getCategoryMaskFromId(CategoryId category) {
		return (category != ANS_TYPE_ALL_ALERTS) ? (1 << (int)category) : ANS_TYPE_MASK_ALL_ALERTS;
	}
	/**
	 * @brief Alert notification control point commands.
	 *
	 * These commands are used to control the behavior of the Alert Notification Service.
	 * They are sent from the client to the server via the Alert Notification Control Point characteristic.
	 *
	 * @note These commands are defined in the Alert Notification Specification.
	 *       UUID: 0x2A44
	 */

	enum CommandId {
		ANS_ENABLE_NEW_INCOMING_ALERT_NOTIFICATION = 0,		 /**< Enable New Incoming Alert Notification.*/
		ANS_ENABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION = 1,	 /**< Enable Unread Category Status Notification.*/
		ANS_DISABLE_NEW_INCOMING_ALERT_NOTIFICATION = 2,	 /**< Disable New Incoming Alert Notification.*/
		ANS_DISABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION = 3, /**< Disable Unread Category Status Notification.*/
		ANS_NOTIFY_NEW_INCOMING_ALERT_IMMEDIATELY = 4,		 /**< Notify New Incoming Alert immediately.*/
		ANS_NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY = 5,	 /**< Notify Unread Category Status immediately.*/
	};
	/**
	 * @brief The ANS control point commands
	 *
	 */
	union control_point_t {
		uint16_t value;
		struct control_point_fields {
            uint8_t command;
			uint8_t category;
		} fields;
	};
	/**
	 * @brief Alert status data structure
	 *
	 */
	union alert_status_t {
		uint16_t value;
		struct alert_status_fields {
			uint8_t category;  //!< The alert category
			uint8_t count;	   //!< The number of unread alerts
		} fields;
	};

   private:
	bool _connected;			   //!< Indicates whether the service is connected to a peer device
	uint32_t _button_press_count;  //!< The number of button press counts since the system startup
	InterruptIn _button;		   //!< The system Button that will be used for capturing alerts
    EventQueue &_event_queue;      //!< A reference to the application's EventQueue

	int _supported_new_alert_category_index;	  //!< The index of the supported new alert category in
												  //!< _characteristics container
	int _supported_unread_alert_category_index;	  //!< The index of the supported unread alert category
												  //!< characteristic in _characteristics container
	int _unread_alert_status_index;				  //!< The index of the unread alert status characteristic in
												  //!< _characteristics container
	int _new_alert_index;						  //!< The index of the new alert characteristic in _characteristics
												  //!< container
	int _alert_notification_control_point_index;  //!< The index of the alert notification control point
												  //!< characteristic in _characteristics container
												  // NOTE: The values of the characteristics

	uint16_t _supported_new_alert_category;		//!< Supported new alerts configuration
	uint16_t _supported_unread_alert_category;	//!< Supported unread alert configuration
	uint16_t _enabled_new_alert_category;		//!< Enabled new alert alerts by the current client
	uint16_t _enabled_unread_alert_category;	//!< Enabled unread alerts by the current client

	alert_status_t _alert_status[ANS_ALERT_CATEGORY_COUNT];	 //!< Alert status for each supported alert

    control_point_t _control_point = {};

   protected:
	/**
	 * @brief Button pressed ISR function.
	 * 1. Detects a button press (discards button release),
	 * 2. Dispatches the button press event to the event queue.
	 *
	 * @note This function is in ISR context: It must be as simple as possible.
	 */
    
    void buttonPressedISR();
    /**
     * @brief Handles the button press event.
     * 
     * @details This function is called when the button is pressed. It adds a new alert to the 
     * `ANS_TYPE_SIMPLE_ALERT` category.
     */
	void buttonPressedHandler();
   public:
	CAlertNotificationService() = delete;
	CAlertNotificationService(const CAlertNotificationService &) = delete;
	CAlertNotificationService &operator=(const CAlertNotificationService &) = delete;

	/**
	 * @brief Construct a new CAlertNotificationService object
	 *
	 * @param button_pin The Button pin that will be used for capturing alerts
	 */
	CAlertNotificationService(EventQueue &event_queue, PinName button_pin);
	/**
	 * @brief Destroy the CAlertNotificationService object
	 *
	 */
	~CAlertNotificationService();

	// = delete; -> this prevents exposing base class member to the caller
	/**
	 * @brief Create a Service object
	 *
	 * @param uuid The UUID of the service
	 * @return ble_error_t indicating if there has been an error in creating the service
	 */
	ble_error_t createService(const char *uuid) = delete;

	/**
	 * @brief Handler, called when the value of a characteristic is modified
	 *
	 * @param characteristic the characteristic that has been modified
	 * @param data The written data
	 * @param size The size of the written data
	 */
	virtual void onDataWrittenHandler(GattCharacteristic *characteristic, const uint8_t *data, uint16_t size) final;

	/**
	 * @brief Handler, called when a peer is connected to the GATT server
	 *
	 */
	virtual void onConnected(void) final;
	/**
	 * @brief Handler, called when a peer is disconnected from the GATT server
	 *
	 */
	virtual void onDisconnected(void) final;

	/**
	 * @brief Registers this service to GattServer instance of the system.
	 *
	 * @param ble A reference to the system BLE instance.
	 */
	virtual void registerService(ble::BLE &ble) final;

	/**
	 * @brief Adds a new alert to the specified category of the alert notification service
	 *
	 * @param category The category of the new alert
	 * @return true if the new alert can be added to the specified category
	 * @return false if the category cannot be added
	 */
	bool addNewAlertToCategory(CAlertNotificationService::CategoryId category);

	/**
	 * @brief Sets the Supported Unread Alerts category value
	 *
	 * @param supportedUnreadAlerts the supported unread alerts bitfield value
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool setSupportedUnreadAlertsCategory(uint16_t supportedUnreadAlerts);
	/**
	 * @brief Adds the specified alert category to the Supported Unread Alerts category value
	 *
	 * @param category The category to be enabled
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool addSupportedUnreadAlertsCategory(CAlertNotificationService::CategoryId category);

	/**
	 * @brief Removes the specified alert category from the supported Unread Alerts category value
	 *
	 * @param category The category to be enabled
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool removeSupportedUnreadAlertsCategory(CAlertNotificationService::CategoryId category);

	/**
	 * @brief Sets the Supported New Alerts category value
	 *
	 * @param supportedUnreadAlerts the supported unread alerts bitfield value
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool setSupportedNewAlertsCategory(uint16_t supportedUnreadAlerts);
	/**
	 * @brief Adds the specified alert category to the Supported New Alerts category value
	 *
	 * @param category The category to be enabled
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool addSupportedNewAlertsCategory(CAlertNotificationService::CategoryId category);

	/**
	 * @brief Removes the specified alert category from the supported New Alerts category value
	 *
	 * @param category The category to be enabled
	 * @return true if peer is not connected
	 * @return false if peer is connected and the category cannot be configured
	 */
	bool removeSupportedNewAlertsCategory(CAlertNotificationService::CategoryId category);

	/**
	 * @brief Clears the alert counts
	 *
	 * @param category Category to clear. if ANS_TYPE_ALL_ALERTS, all alert types are cleared.
	 */
	void clearAlertsOfCategory(CategoryId category);
};
