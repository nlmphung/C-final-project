#ifndef _BLE_UTILS_H_
#define _BLE_UTILS_H_

#include <BLE.h>
#include <iomanip> // for std::setw, std::setfill, std::hex
#include <sstream> // for std::stringstream

/**
 * @brief This file contains definitions of utility functions for common Bluetooth Low Energy (BLE) operations.
 * These functions provide helper functionality for formatting and converting data related to BLE addresses,
 * buffers, passkeys, and error codes.  See individual function documentations for details.
 */

/**
 * @brief Converts a Bluetooth address to a human-readable hex string.
 *
 * The address is formatted as a colon-separated hex string (e.g., "00:11:22:33:44:55").
 *
 * @param address The Bluetooth address to convert.
 * @return The address as a human-readable hex string.
 */
std::string bluetoothAddressToString(const ble::address_t& address);

/**
 * @brief Converts a Bluetooth device's address and type to a human-readable string.
 *
 * Includes the address type (PUBLIC, RANDOM, etc.) in the output string.
 *
 * @param type The type of the Bluetooth address (e.g., `ble::own_address_type_t::PUBLIC`).
 * @param address The Bluetooth address to convert.
 * @return A human-readable string containing the address type and the address itself.
 */
std::string bluetoothAddressToString(const ble::own_address_type_t type, const ble::address_t& address);

/**
 * @brief Converts a peer Bluetooth device's address and type to a human-readable string.
 *
 * Includes the address type (PUBLIC, RANDOM, etc.) in the output string.
 *
 * @param type The type of the peer Bluetooth address (e.g., `ble::peer_address_type_t::PUBLIC`).
 * @param address The Bluetooth address to convert.
 * @return A human-readable string containing the address type and the address itself.
 */
std::string bluetoothAddressToString(const ble::peer_address_type_t type, const ble::address_t& address);

/**
 * @brief Converts a byte buffer to a colon-separated hex string.
 *
 * Useful for debugging and displaying byte array data.
 *
 * @param buffer The byte buffer to convert.
 * @param size The size of the buffer in bytes.
 * @return A colon-separated hex string representation of the buffer.
 */
std::string bufferToHex(const uint8_t* buffer, size_t size);

/**
 * @brief Converts a passkey (SecurityManager::Passkey_t) to a human-readable string.
 *
 * The passkey is formatted as a space-separated string of characters.
 *
 * @param passkey The passkey to convert.
 * @return A human-readable string representation of the passkey.
 */
std::string passKeyToString(const SecurityManager::Passkey_t passkey);

/**
 * @brief Converts a Mbed OS BLE error code to a human-readable string.
 *
 * Provides a more descriptive error message than the raw error code.
 *
 * @param error The BLE error code.
 * @param message An optional message to prepend to the error string.
 * @return A human-readable string describing the BLE error.  Includes the error code and a description.
 */
std::string bleErrorToString(ble_error_t error, const char* message = nullptr);

#endif	//! _BLE_UTILS_H_
