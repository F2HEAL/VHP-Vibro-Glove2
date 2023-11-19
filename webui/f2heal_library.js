

/**
 * Global constants - Matches defenitions in Message.hpp
 */

const MESSAGE_TYPE_VOLUME = 1;
const MESSAGE_TYPE_TOGGLE = 2;
const MESSAGE_TYPE_CHANNEL_TEST = 3;
const MESSAGE_TYPE_8_CHANNEL = 4;
const MESSAGE_TYPE_STIM_FREQ = 5;
const MESSAGE_TYPE_STIM_DURATION = 6;
const MESSAGE_TYPE_CYCLE_PERIOD = 7;
const MESSAGE_TYPE_PAUZE_CYCLE_PERIOD = 8;
const MESSAGE_TYPE_PAUZED_CYCLES = 9;
const MESSAGE_TYPE_JITTER = 10;
const MESSAGE_TYPE_TEST_MODE = 11;
const MESSAGE_TYPE_STATUS_BATCH = 12;
const MESSAGE_TYPE_GET_STATUS_BATCH = 13;
const MESSAGE_TYPE_GET_VOLUME = 14;
const MESSAGE_TYPE_SETTINGS_BATCH = 15;
const MESSAGE_TYPE_GET_SETTINGS_BATCH = 16;


/** Function that does nothing, for use as a default UI function. */
function noOp() {
  return;
}


/**
 * Connects BLE to device that both has a name starting with 'Audio-to-Tactile'
 * and is advertising the Nordic UART Service, after user input.
 * On a successful connection, sends a "get tuning" request.
 * Must be a stand-alone function, outside of a class.
 *
 * @param {!BLEManager} bleManager  An instance of the BLEManager class that
 *    will store BLE state created or modified in this function.
 */
function connect(bleManager) {
    // UUIDs for the BLE Nordic UART Service (NUS). We use NUS to send messages
    // between this web app and the device.
    //
    // Reference:
    // https://infocenter.nordicsemi.com/index.jsp?topic=%2Fsdk_nrf5_v16.0.0%2Fble_sdk_app_nus_eval.html
    const NUS_SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
    const NUS_RX_CHARACTERISTIC_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
    const NUS_TX_CHARACTERISTIC_UUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
    let nusService;
    if (!navigator.bluetooth) {
	bleManager.log('WebBluetooth API is not available.\r\n' +
		       'Please make sure the Web Bluetooth flag is enabled.');
	return;
    }
    navigator.bluetooth.requestDevice({  // Scan for a matching BLE device.
	filters: [
	    {namePrefix: 'Audio-to-Tactile'},
	    {services: [NUS_SERVICE_UUID]},
	],
	optionalServices: [
	    NUS_SERVICE_UUID,
	    NUS_RX_CHARACTERISTIC_UUID,
	    NUS_TX_CHARACTERISTIC_UUID
	],
    })
	.then(device => {
	    bleManager.bleDevice = device;
	    bleManager.bleDevice.addEventListener('gattserverdisconnected',
						  bleManager.onDisconnected.bind(bleManager));
	    // Connect to the GATT server.
	    return device.gatt.connect();
	})
	.then(server => {
	    // Locate the NUS service.
	    service = server.getPrimaryService(NUS_SERVICE_UUID);
	    return service;
	}).then(service => {
	    nusService = service;
	    // Locate Rx characteristic.
	    return nusService.getCharacteristic(NUS_RX_CHARACTERISTIC_UUID);
	})
	.then(characteristic => {
	    bleManager.nusRx = characteristic;
	    // Locate Tx characteristic.
	    return nusService.getCharacteristic(NUS_TX_CHARACTERISTIC_UUID);
	})
	.then(characteristic => {
	    bleManager.nusTx = characteristic;
	    // Listen for messages sent from the device.
	    bleManager.nusTx.addEventListener('characteristicvaluechanged',
					      bleManager.onReceivedMessage.bind(bleManager));
	    return bleManager.nusTx.startNotifications();
	})
	.then(() => {
	    bleManager.log('BLE connected to ' + bleManager.bleDevice.name);
	    bleManager.connected = true;
	    bleManager.onConnectionUIUpdate(bleManager.connected);
	    // Send "get settings batch" request to the device.
	    //bleManager.requestStatusBatch();
	    bleManager.requestSettingsBatch();
	})
	.catch(error => {
	    bleManager.log('Error: ' + error);
	    if (bleManager.bleDevice && bleManager.bleDevice.gatt.connected) {
		bleManager.bleDevice.gatt.disconnect();
	    }
	});
}

class BleManager {
    constructor(loggingFunction=noOp,
		OnConnectionUIUpdate=noOp,
		volumeUpdate=noOp,
		onStreamUpdate=noOp,
	       onSettingsBatch=noOp) {
	this.log = loggingFunction;
	this.onConnectionUIUpdate = OnConnectionUIUpdate;
	this.volumeUpdate = volumeUpdate;
	this.onStreamUpdate = onStreamUpdate;
	this.onSettingsBatch = onSettingsBatch;

	this.bleDevice = null;
	this.nusRx = null;
	this.nusTx = null;
	this.connected = false;

	//variable to hold status
	this.a_running = false;
	this.a_runningsince = 0;
	this.a_battery = 0.0;

	
	// variables to hold settings
	this.s_chan8 = false;
	this.s_stimfreq = 0;
	this.s_stimduration = 0;
	this.s_cycleperiod = 0;
	this.s_pauzecycleperiod = 0;
	this.s_pauzedcycles = 0;
	this.s_jitter = 0;
	this.s_testmode = false;
    }

    /** Toggle the BLE connection. */
    connectionToggle() {
	if (this.connected) {
	    this.disconnect();
	} else {
	    connect(this);
	}
    }

    /** Disconnect the BLE device and update the UI. */
    disconnect() {
	if (this.bleDevice && this.bleDevice.gatt.connected) {
	    this.bleDevice.gatt.disconnect();
	    this.connected = false;
	    this.onConnectionUIUpdate();
	}
    }

    /**
     * Update the connected flag when disconnected, then update the UI.
     * @private
     */
    onDisconnected() {
	this.connected = false;
	this.log('BLE disconnected.');
	this.onConnectionUIUpdate();
    }

    /**
     * Toggle the stream state
     */
    streamToggle() {
	if(!this.connected) { return; }
	this.log("Toggle stream");	
	this.writeMessage(MESSAGE_TYPE_TOGGLE, new Uint8Array(0));
    }
    
    /**
     * Handles a volume message received from the device.
     */
    receiveVolume(messagePayload) {
	this.log("Receive Volume (" + messagePayload + ")");
	let value = new Uint8Array(messagePayload.buffer);
	this.volumeUpdate(value[0]);

	this.requestStatusBatch();
    }
    
    /**
     * Handles the settings batch message from the device
     *
     * Matches the function Message::WriteSettings()
     */
    receiveSettingsBatch(messagePayload) {
	this.log("Receive Settings Batch (" + messagePayload + ")");

	let view_chan8 = new DataView(messagePayload.buffer, 0, 1);
	let val_chan8 = view_chan8.getUint8(0);
	this.s_chan8 = val_chan8 == 1;

	
	let view_sf = new DataView(messagePayload.buffer, 1, 4);
	this.s_stimfreq = view_sf.getUint32(0, /*littleEndian=*/true);

	let view_sd = new DataView(messagePayload.buffer, 5, 4);
	this.s_stimduration = view_sd.getUint32(0, /*littleEndian=*/true);

	let view_cp = new DataView(messagePayload.buffer, 9, 4);
	this.s_cycleperiod = view_cp.getUint32(0, /*littleEndian=*/true);

	let view_pcp = new DataView(messagePayload.buffer, 13, 4);
	this.s_pauzecycleperiod = view_pcp.getUint32(0, /*littleEndian=*/true);

	let view_pc = new DataView(messagePayload.buffer, 17, 4);
	this.s_pauzedcycles = view_pc.getUint32(0, /*littleEndian=*/true);

	let view_j = new DataView(messagePayload.buffer, 21, 4);
	this.s_jitter = view_j.getUint32(0, /*littleEndian=*/true);

	let view_tm = new  DataView(messagePayload.buffer, 25, 1);
	let val_tm = view_tm.getUint8(0);
	this.s_testmode = val_tm == 1;

	this.onSettingsBatch();
	
	this.log(" Settings 8chan: " + this.s_chan8
		 + ", stimfreq: " + this.s_stimfreq
		 + ", stimduration: " + this.s_stimduration
		 + ", cycleperiod: " + this.s_cycleperiod
		 + ", pauzecycleperiod: " + this.s_pauzecycleperiod
		 + ", pauzedcycles: " + this.s_pauzedcycles
		 + ", jitter: " + this.s_jitter
		 + ", testmode: " + this.s_testmode);



	
	// Now that we've received the settings,
	// we want to send a "get volume" request to the device.	    
	this.requestGetVolume();
    }
				

    /**
     * Handles the status batch message from the device
     *
     * Matches the function Message::WriteStatus()
     */
    receiveStatusBatch(messagePayload) {
	this.log("Receive Status Batch (" + messagePayload + ")");

	let view_running = new DataView(messagePayload.buffer, 0, 1);
	let val_running = view_running.getUint8(0);
	this.a_running = val_running == 1;

	let view_runningsince = new DataView(messagePayload.buffer, 1, 8);
	this.a_runningsince = view_runningsince.getBigUint64(0, /*littleEndian=*/true);

	let view_battery = new DataView(messagePayload.buffer, 9, 4);
	this.a_battery = view_battery.getFloat32(0, /*littleEndian=*/true);

	this.onStreamUpdate();

	this.log(" Status running: " + this.a_running
		 + ", runningsince: " + this.a_runningsince
		 + ", battery: " + this.a_battery);


    }
    
    /**
     * Send a request to the device for the current Volume
     */
    requestGetVolume() {
	if(!this.connected) { return; }
	this.log("Request Get Volume");
	this.writeMessage(MESSAGE_TYPE_GET_VOLUME, new Uint8Array(0));
    }

    /**
     * Send a request for settings batch to the device
     */
    requestSettingsBatch() {
	if(!this.connected) { return; }
	this.log("Request Get Settings Batch");
	this.writeMessage(MESSAGE_TYPE_GET_SETTINGS_BATCH, new Uint8Array(0));
    }

    requestStatusBatch() {
	if(!this.connected) { return; }
	this.log("Request Get Status Batch");
	this.writeMessage(MESSAGE_TYPE_GET_STATUS_BATCH, new Uint8Array(0));
    }

	
    /**
     * Send a new value for Volume to the device
     */
    setVolume(num) {
	if(!this.connected) { return; }
	this.log("Setting volume to: " + num);
	let buffer = new Uint8Array(1);
	buffer[0] = num;
	this.writeMessage(MESSAGE_TYPE_VOLUME, buffer);
    }

    /**
     * Send new message with bool payload
     */
    setMessageBool(message, val) {
	if(!this.connected) { return; }
	this.log("Set Message (" + message + ") Bool (" + val + ")");	
	let buffer = new Uint8Array(1);
	buffer[0] = val ? 1 : 0;
	this.writeMessage(message, buffer);
    }



    /**
     * Send a new message with uint32 payload
     */

    setMessageUInt32(message, num) {
	if(!this.connected) { return; }
	this.log("Set Message (" + message + ") UInt32 (" + num + ")");
	
	let arr = new Uint8Array(4);
	new DataView(arr.buffer).setUint32(0, num, /*littleEndian*/ true);
	this.writeMessage(message, arr);
    }
    
    
    /**
     * Handles a new BLE message from the device by parsing message type and
     * calling the appropriate handler.
     * @param {!Event} event Event containing message information.
     * @private
     */
    onReceivedMessage(event) {
	let bytes = event.target.value;
	if (bytes.byteLength < 4 || bytes.byteLength != 4 + bytes.getUint8(3)) {
	    this.log('Received invalid message.');
	}
	let messageType = bytes.getUint8(2);
	let messagePayload = new Uint8Array(bytes.getUint8(3));
	for (let i = 0; i < messagePayload.byteLength; i++) {
	    messagePayload[i] = bytes.getUint8(4 + i);
	}
	this.log('Got type: ' + messageType + ', [' +
		 messagePayload.join(', ') + ']');
	switch (messageType) {
	case MESSAGE_TYPE_VOLUME:
	    this.receiveVolume(messagePayload);
	    break;
	case MESSAGE_TYPE_SETTINGS_BATCH:
	    this.receiveSettingsBatch(messagePayload);
	    break;
	case MESSAGE_TYPE_STATUS_BATCH:
	    this.receiveStatusBatch(messagePayload);
	    break;
	default:
	    this.log('Unsupported message type.');
	}
    }

    /**
     * Writes a message to the device.
     * @param {number} messageType Code indicating the message type.
     * @param {!Uint8Array} messagePayload Contents to send to device.
     * @private
     */
    writeMessage(messageType, messagePayload) {
	if (!this.connected) { return; }
	this.log('writeMessage: Sent type: ' + messageType + ', [' +
		 messagePayload.join(', ') + ']');
	let bytes = new Uint8Array(4 + messagePayload.byteLength);
	bytes[2] = messageType;
	bytes[3] = messagePayload.byteLength;
	for (let i = 0; i < messagePayload.byteLength; i++) {
	    bytes[4 + i] = messagePayload[i];
	}
	// Compute Fletcher-16 checksum.
	let sum1 = 1;
	let sum2 = 0;
	for (let i = 2; i < bytes.length; i++) {
	    sum1 += bytes[i];
	    sum2 += sum1;
	}
	bytes[0] = sum1 % 255;
	bytes[1] = sum2 % 255;
	this.nusRx.writeValue(bytes);
    }
}



