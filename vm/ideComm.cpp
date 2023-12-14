/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// ideComm.cpp - Primitives to communicate with the MicroBlocks IDE.
// John Maloney, December 2023

#include <Arduino.h>

#include "mem.h"
#include "interp.h"

#if defined(BLE_IDE)

// BLE Communications

#include <NimBLEDevice.h>

static BLEServer *pServer = NULL;
static BLECharacteristic *pTxCharacteristic;
static BLECharacteristic *pRxCharacteristic;
static bool ideConnected = false;

// BLE_SEND_MAX - maximum bytes to send in a single attribute write (max is 512)
// INTER_SEND_TIME - don't send data more often than this to avoid NimBLE error & disconnect
#define BLE_SEND_MAX 250
#define INTER_SEND_TIME 20
static uint32 lastSendTime = 0;
static int lastRC = 0;

#define RECV_BUF_MAX 1024
static uint8_t bleRecvBuf[RECV_BUF_MAX];
static int bleBytesAvailable = 0;
static int overRuns = 0;

#define SERVICE_UUID			"6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // Nordic UART service
#define CHARACTERISTIC_UUID_RX	"6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX	"6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// BLE Helper Functions

static void reportNum2(const char *label, int n) {
	Serial.print(label);
	Serial.print(": ");
	Serial.println(n);
}

static void bleReceiveData(const uint8_t *data, int byteCount) {
	int available = RECV_BUF_MAX - bleBytesAvailable;
	if (byteCount > available) {
		overRuns++;
		byteCount = available;
	}

	memcpy(&bleRecvBuf[bleBytesAvailable], data, byteCount);
	bleBytesAvailable += byteCount;
}

static int bleSendData(uint8_t *data, int byteCount) {
	if (bleSendData <= 0) return 0;

	// do not send more often than INTER_SEND_TIME msecs
	uint32 now = millisecs();
	if (lastSendTime > now) lastSendTime = 0; // clock wrap
	if ((now - lastSendTime) < INTER_SEND_TIME) return 0;

	// send byteCount bytes
	if (byteCount > BLE_SEND_MAX) byteCount = BLE_SEND_MAX;
	lastRC = 0; // will be set to non-zero if notify() call fails
	pTxCharacteristic->setValue(data, byteCount);
	pTxCharacteristic->notify();
	if (lastRC != 0) {
		byteCount = 0; // write+notify failed; retry later
	}

	lastSendTime = now;
	return byteCount;
}

class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
		ideConnected = true;
	}
	void onDisconnect(BLEServer* pServer) {
		ideConnected = false;
	}
};

class MyCallbacks: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic, ble_gap_conn_desc* desc) {
		// Handle incoming BLE data.

		NimBLEAttValue value = pCharacteristic->getValue();
		bleReceiveData(value.data(), value.length());
	}
	void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
		// Record the last return code. This is used to tell when a notify() has failed
		// (because there are no buffers) so that it can be re-tried later.

		lastRC = code;
	}
};

// Start BLE

void startBLE_UART() {
	// Create BLE Device
	BLEDevice::init("MicroBlocks BLE");

	// Create BLE Server
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create BLE Service
	BLEService *pService = pServer->createService(SERVICE_UUID);

	// Create BLE Characteristics
	pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
	pTxCharacteristic->setCallbacks(new MyCallbacks());
	pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE_NR);
	pRxCharacteristic->setCallbacks(new MyCallbacks());

	// Start the service
	pService->start();

	// Add the service to the advertisment data
	pServer->getAdvertising()->addServiceUUID(pService->getUUID());

	// Start advertising
	pServer->getAdvertising()->start();
	Serial.println("MicroBlocks BLE Started");
}

int recvBytes(uint8 *buf, int count) {
	int bytesRead = (count < bleBytesAvailable) ? count : bleBytesAvailable;
	if (bytesRead == 0) return 0;

	memcpy(buf, bleRecvBuf, bytesRead); // copy bytes to buf

	int remainingBytes = bleBytesAvailable - bytesRead;
	if (remainingBytes > 0) {
		// remove bytesRead bytes from bleRecvBuf
		memcpy(bleRecvBuf, &bleRecvBuf[bytesRead], remainingBytes);
	}
	bleBytesAvailable = remainingBytes;

	return bytesRead;
}

int sendBytes(uint8 *buf, int start, int end) {
	// Send bytes buf[start] through buf[end - 1] and return the number of bytes sent.

	return bleSendData(&buf[start], end - start);
}

void restartSerial() {
	// Noop when using BLE
}

#else

// Serial Communications

int recvBytes(uint8 *buf, int count) {
	int bytesRead = Serial.available();
	if (bytesRead > count) bytesRead = count; // there is only enough room for count bytes
	return Serial.readBytes(buf, bytesRead);
}

int sendBytes(uint8 *buf, int start, int end) {
	// Send bytes buf[start] through buf[end - 1] and return the number of bytes sent.

	return Serial.write(&buf[start], end - start);
}

void restartSerial() {
	// Needed to work around a micro:bit issue that Serial can lock up during Flash compaction.

	Serial.end();
	Serial.begin(115200);
}

#endif
