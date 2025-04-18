#pragma once
#include "ble_utils.h"
#include <BLE.h>
#include <mbed.h>

#include <memory>
#include <vector>
#include <iostream>

/**
 * @brief This class provides a robust framework for managing GATT characteristics within a GATT service. It simplifies 
 * creating and managing services with multiple characteristics, ensuring data integrity and efficient memory management. 
 * The class utilizes C++ containers, smart pointers, and templates for efficient and type-safe operations.
 *
 * @details The `CGattService` class offers a high-level abstraction for interacting with GATT characteristics, providing 
 * the following key functionalities:
 *
 *   - **Characteristic Management:** Adds characteristics to the service, defining their UUIDs, properties (read, write, 
 *     notify, indicate), and optional user descriptions. Uses `std::vector` for efficient storage and access.
 *   - **Type-Safe Data Handling:** Employs C++ templates to provide type-safe access to characteristic values, preventing 
 *     common data type errors and improving code maintainability.
 *   - **Secure Data Access:** Provides template functions (`getCharacteristicValue`, `setCharacteristicValue`) for reading 
 *     and writing characteristic values directly to the GATT server, ensuring data integrity and managing potential buffer 
 *     overflow errors. This approach avoids using local copies of characteristic values, enhancing data consistency.
 *   - **Service Creation and Registration:** Creates the GATT service instance using a provided UUID and registers it with 
 *     the BLE stack. Manages the lifecycle of the GATT service object using `std::unique_ptr` for automatic resource cleanup.
 *   - **Event Handling:** Defines a pure virtual function `onDataWrittenHandler` for handling data write events. Derived 
 *     classes *must* implement this function to define custom behavior when a characteristic is written to.
 *   - **Connection Management:** Includes pure virtual functions `onConnected` and `onDisconnected` to handle connection 
 *     and disconnection events, allowing derived classes to implement service-specific actions.
 * 
 * IMPORTANT::
 * This class uses C++ containers, C++ templates, and smart pointers.
 * If you are not familiar with these, first check the container Module in Part I.
 * For templates and smart pointers, see the Advanced C++ concepts module.
 *
 * @note std::vector stores its elements in contiguous memory region. Therefore,
 * an address of its first element can be used as an C array.
 *
 * @note Mbed OS GattService and GattCharacteristic classes have some non-trivial
 * assumptions about their member life spans. For example, when constructing a characteristic
 * attributes are copied from the constructor arguments, but later they are  copied and/or moved
 * to the memory that is managed by the stack. Therefore, it is developer's responsibility to
 * make sure that the constructor arguments are valid till the stack moves/copies the
 * arguments to the stack.
 *
 * This class ensures that the memory remains intact.
 *
 * NOTE: This class cannot be copied since _service is stored as std::unique_ptr
 * The `CGattService` class is designed to be inherited from, providing a flexible and extensible framework for creating 
 * custom GATT services. It is non-copyable to ensure proper management of the `std::unique_ptr` member.
 */
class CGattService : mbed::NonCopyable<CGattService> {
protected:
    std::vector<std::vector<GattAttribute *>>
        _characteristics_user_descriptions;	 //!< The user description attributes of all the characteristics
    std::vector<GattCharacteristic *> _characteristics;	 //!< The characteristics of the service

    std::unique_ptr<GattService> _service;	//!< The service encapsulated in unique pointer
public:
    /**
     * @brief Construct a new CGattService object
     * Initially the service is empty
     */
    CGattService();
    /**
     * @brief Destroy the CGattService object
     * Releases the service and deallocates all characteristics and their descriptors.
     */
    ~CGattService();

    /**
     * @brief Adds an empty characteristic to the service if the service has not been created
     *
     * @details The created characteristic does not have a value
     *
     * @param uuid The UUID of the characteristic
     * @param properties The properties of the characteristic. It does not have any properties by default.
     * @param user_description The user description of the characteristic. It does not have a user description
     * by default.
     * @param value The initial value of the characteristic
     * @param max_value_size The maximum value of the characteristic. If value is not nullptr, its size must
     * be equal to max_value_size.
     * @return true If the characteristic can be created and added (when service has not been created yet)
     * @return false If the service is created already or there is an error in creating the characteristic
     */
    bool addCharacteristic(const UUID &uuid,
                            const uint8_t properties = GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NONE,
                            const char *user_description = nullptr,
                            uint8_t *value = nullptr,
                            size_t max_value_size = 0);

    /**
     * @brief Adds a characteristic to the service if the service has not been created
     *
     * @tparam T The type of the characteristic value
     * @param uuid The UUID of the characteristic
     * @param value The default value of the characteristic
     * @param properties The properties of the characteristic. It does not have any properties by default.
     * @param user_description The user description of the characteristic. It does not have a user description
     * by default.
     * @return true If the characteristic can be created and added (when service has not been created yet)
     * @return false If the service is created already or there is an error in creating the characteristic
     */
    template <typename T>
	bool addCharacteristic(const char *uuid,
						   const T &value,
						   const uint8_t properties = GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NONE,
						   const char *user_description = nullptr) {
		// TODO:: Implement this functions

		// 1. check whether the service has not been created yet.
		if (_service) {
			// if it is, we cannot add characteristics to it.
			return false;
		}
		// 2. Call the base class addCharacteristic for the value type
		//  i. the value is address of value argument
		//  ii. the value size is size of T
		//  iii. return the return value of the base class addCharacteristic function
        bool success = addCharacteristic(uuid,
                          properties,
                          user_description,
                          const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&value)),
                          sizeof(T));
		
		return success;
		
	}

    /**
     * @brief Get the Characteristic that has the specified Value attribute Handle
     *
     * @note This function must be called only if BLE is successfully initialized
     *
     * @param value_handle The handle of the characteristics value attribute
     * @return A pointer to the characteristics. nullptr if a characteristics
     * with the specified value handle cannot be found.
     */
    GattCharacteristic *getCharacteristicWithValueHandle(const GattAttribute::Handle_t &value_handle) const;

    /**
     * @brief Returns the user description of the characteristic with the specified value handle
     *
     * @param value_handle The value handle of the characteristic
     * @return nullptr if the characteristic is not in this service or its user description is not set;
     * otherwise, a pointer to the user description string
     */
    const char *getCharacteristicUserDescription(const GattAttribute::Handle_t &value_handle) const;

    /**
     * @brief Handler, called when the value of a characteristic is modified
     * 
     * @note This function must be implemented by the derived class.
     *
     * @param characteristic The characteristic that has been modified
     * @param data The written data
     * @param size The size of the written data
     */
    virtual void onDataWrittenHandler(GattCharacteristic *characteristic,
                                      const uint8_t *data,
                                      uint16_t size) = 0;

    /**
     * @brief Create a Service object
     *
     * @param uuid The UUID of the service
     * @return ble_error_t indicating if there has been an error in creating the service
     */
    ble_error_t createService(const UUID &uuid);

    /**
     * @brief Get a shared pointer to the GattService of this object
     *
     * @return A pointer to the GattService of this object. nullptr if the service has not been created.
     */
    GattService *getService() const;

    /**
     * @brief Get the Characteristic Count added to this service
     *
     * @return The number of characteristics
     */
    unsigned int getCharacteristicCount() const;

    /**
     * @brief Returns whether this service has been created
     *
     * @return true if service has been created
     * @return false if service has not been created
     */
    explicit operator bool() const {
        // Use the bool operator of std::unique_ptr
        return bool(_service);
    }

    /**
     * @brief Registers this service to GattServer instance of the system.
     * 
     * @note This function 
     *
     * @param ble A reference to the system BLE instance.
     */
    virtual void registerService(ble::BLE &ble);

    /**
     * @brief On connection completed handler function of the service
     * 
     * @note This function must be implemented by the derived class.
     *
     */
    virtual void onConnected(void) = 0;

    /**
     * @brief On disconnected from the client handler function of the service
     * 
     * @note This function must be implemented by the derived class.
     *
     */
    virtual void onDisconnected(void) = 0;

    /**
     * @brief Get the Characteristic Value
     *
     * @note This function does NOT work on local copies of the Value attributes
     *
     * @tparam T The characteristic value type
     * @param characteristic The characteristic
     * @param value A reference to the value
     * @return The error received while reading the characteristic value.
     */
    template <typename T>
    static ble_error_t getCharacteristicValue(GattCharacteristic &characteristic, T &value) {
		ble::GattServer &server = ble::BLE::Instance().gattServer();
        
		// 1. Read the value of the characteristic using GattServer::read function 
		//    i. declare size variable that is initialized to size of T data type
		//    ii. call GattServer::read function
		//    iii. You must store the return code of the call
        // 
        size_t size = sizeof(T);
        uint8_t buffer[size];
        uint16_t *out = (uint16_t *) size;
        
        ble_error_t error = server.read(characteristic.getValueAttribute().getHandle(),  buffer, out);
        // NOTE: The second argument of the GattServer::read function is a pointer to the value of the characteristic.
        //       You must use reinterpret_cast operator to cast the T* to uint8_t* to use value as
        //       destination buffer. This is required since unlike C, C++ does not allow implicit
        //       conversion between pointers of different types.
        // 
		// IMPORTANT::: `GattServer::read` function uses `lengthP` parameter (third argument) for two purposes.
		//  I. To indicate the maximum size that buffer parameter can store (in this case it must be sizeof(T))
		//  II. To return the actual size of the value so that the next call would have a larger size. So, 
        //      you must declare a local variable of type uint16_t and pass its address to the function.
		

		// 2. If return code indicates no error, check the value of lengthP argument of GattServer::read
		//    i. If returned value of lengthP is larger than size of T, return BLE_ERROR_BUFFER_OVERFLOW
		//    ii. Otherwise return the returned error code.
		if (error != ble_error_t::BLE_ERROR_NONE) {
	        std::cout << bleErrorToString(error, "getCharacteristicValue error") << std::endl;
            return error;
        }

        if (*out > size) {
            return BLE_ERROR_BUFFER_OVERFLOW;
        }
        return error;
		
    }

    /**
     * @brief Set the Characteristic Value
     *
     * @note This function does NOT work on local copies of the Value attributes
     *
     * @tparam T The characteristic value type
     * @param characteristic The characteristic
     * @param value A reference to the value
     * @return The error received while writing the characteristic value.
     */
    template <typename T>
    static ble_error_t setCharacteristicValue(GattCharacteristic &characteristic, T &value) {
		ble::GattServer &server = ble::BLE::Instance().gattServer();
		// 1. Write the value of the characteristic using GattServer::write function
		//    i. call GattServer::write function. Note that localOnly parameter (third argument) must be false since 
        //       we want to write the value to the server and not just locally.
		//    ii. We must use reinterpret_cast operator to cast the T* to const uint8_t* to use value as
		//        destination buffer for the reason mentioned in the getCharacteristicValue function.
		//    iii. Return the returned error code
		const uint8_t *value_ptr = reinterpret_cast<const uint8_t *>(&value);
        uint16_t value_size = sizeof(T);
        ble_error_t error = server.write(characteristic.getValueHandle(), value_ptr, value_size, false);

        return error;
		
    }
};
