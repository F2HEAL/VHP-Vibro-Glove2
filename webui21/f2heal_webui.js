// Function to get URL parameter by name
function getUrlParameter(name) {
  const urlParams = new URLSearchParams(window.location.search);
  return urlParams.get(name);
}

// Function to dynamically load presets from a JSON file and apply a preset if specified in the URL
function loadPresets() {
  fetch('presets.json')
    .then(response => response.json())
	.then(data => {
      const presetSelect = document.getElementById('presetSelect');
      
      // Add a disabled dummy option as the first entry
      const dummyOption = document.createElement('option');
      dummyOption.text = "Select an option";
      dummyOption.value = "";
      dummyOption.disabled = true;
      dummyOption.selected = true; 
      presetSelect.add(dummyOption);
      
      data.presets.forEach(preset => {
        const option = document.createElement('option');
        option.value = preset.name;
        option.text = preset.name;
        option.dataset.values = JSON.stringify(preset);
        presetSelect.add(option);
      });

      // Check URL for preset parameter and apply if present
      const presetParam = getUrlParameter('preset');
      if (presetParam) {
        const preset = data.presets.find(p => p.name === presetParam);
        if (preset) {
          applyPresetValues(preset);
          presetSelect.value = presetParam; // Set the select to the current preset
        } else {
          console.error(`Preset "${presetParam}" not found.`);
          alert(`Preset "${presetParam}" not found.`);
        }
      }

    })
    .catch(error => console.error('Error loading presets:', error));
}



// function loadPresets() {
//     fetch('presets.json')
//         .then(response => response.json())
//         .then(data => {
//             const presetSelect = document.getElementById('presetSelect');

// 	    // Add a disabled dummy option as the first entry
// 	    const dummyOption = document.createElement('option');
// 	    dummyOption.text = "Select an option";
// 	    dummyOption.value = "";
// 	    dummyOption.disabled = true;
// 	    dummyOption.selected = true; 
// 	    presetSelect.add(dummyOption);
	    
//             data.presets.forEach(preset => {
//                 const option = document.createElement('option');
//                 option.value = preset.name;
//                 option.text = preset.name;
//                 option.dataset.values = JSON.stringify(preset.values);
//                 presetSelect.add(option);
//             });
//         }).catch(error => console.error('Error loading presets:', error));
// }


// WGO: It should be possible to load the presets.json only once and
// re-use the values from there on. There is a problem with that, for
// which I lack the JavaScript knowledge to fix it. Any help appreciated! 
function loadAndApplyPreset(presetName) {
    fetch('presets.json')
	.then(response => response.json())
	.then(data => {
	    const preset = data.presets.find(p => p.name === presetName);
	    
	    if (preset) {
		applyPresetValues(preset);
	    } else {
		console.error(`Preset "${presetName}" not found.`);
	    }
	})
	.catch(error => console.error('Error loading presets:', error));
}

// A rather dirty hack to allow the BLE commands to have been
// executed, a better solution would be to make a batch send function
function sleep(milliseconds) {
  return new Promise(resolve => setTimeout(resolve, milliseconds));
}

// Another one, add an extra sleep for applying the preset when
// connecting to a device
async function slowApplyPresetValues(preset) {
    await sleep(1000);
    applyPresetValues(preset);
}

// Function to apply a preset's values
async function applyPresetValues(preset) {
    console.log("Applying preset values:", preset);
    // Assuming you have a function to set each parameter in your environment
    bleInstance.setVolume(preset.volume_percent); await sleep(150);
    bleInstance.setMessageBool(MESSAGE_TYPE_8_CHANNEL, preset['8_channels']); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_STIM_FREQ, preset.stimulation_frequency_hz); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_STIM_DURATION, preset.stimulation_duration_ms); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_CYCLE_PERIOD, preset.cycle_period_duration_ms); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_PAUZE_CYCLE_PERIOD, preset.number_of_cycles_in_pauze_cycle); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_PAUZED_CYCLES, preset.number_of_cycles_in_pauze_cycle_to_pauze); await sleep(150);
    bleInstance.setMessageUInt32(MESSAGE_TYPE_JITTER, preset.jitter_on_timing_permill); await sleep(150);

    // Update UI elements
    document.getElementById('s_chan8').checked = preset['8_channels'];
    document.getElementById('s_volume').value = preset.volume_percent;
    document.getElementById('s_volume_text').value = preset.volume_percent;
    document.getElementById('s_stimfreq').value = preset.stimulation_frequency_hz;
    document.getElementById('s_stimdur').value = preset.stimulation_duration_ms;
    document.getElementById('s_cycleperiod').value = preset.cycle_period_duration_ms;
    document.getElementById('s_pauzecycleperiod').value = preset.number_of_cycles_in_pauze_cycle;
    document.getElementById('s_pauzedcycles').value = preset.number_of_cycles_in_pauze_cycle_to_pauze;
    document.getElementById('s_jitter').value = preset.jitter_on_timing_permill;
}

function toggleSection(sectionId) {
    const section = document.getElementById(sectionId);
    section.style.display = (section.style.display === 'none') ? 'block' : 'none';
}

function validateAndSetUInt32(message, element, min, max) {
    const value = parseInt(element.value, 10);
    if (value >= min && value <= max) {
        bleInstance.setMessageUInt32(message, value);
    } else {
        alert(`Value must be between ${min} and ${max}.`);
        element.value = element.dataset.prevValue || min;
    }
}

function validatePauzedCycles(element) {
    const value = parseInt(element.value, 10);
    const max = parseInt(document.getElementById('s_pauzecycleperiod').value, 10);
    if (value >= 0 && value < max) {
        bleInstance.setMessageUInt32(MESSAGE_TYPE_PAUZED_CYCLES, value);
    } else {
        alert('Pauzed Cycles must be greater than or equal to 0 and less than Pauze Cycle Period.');
        element.value = element.dataset.prevValue || 0;
    }
}

var logDisplay = document.getElementById('logDisplay');
var logLines = [];
/**
 * Appends a message to the log window. A newline is inserted after
 * the message.
 * @param {string} s A string to be displayed in the log window.
 */
var logFunction = function log(s) {
    // If the log gets long, discard old messages to keep UI responsive.
    var discard = (logLines.length >= 100);
    if (discard) {
	logLines = logLines.slice(-99);
    }
    var now = new Date();
    var timestamp = ('00' + now.getHours()).slice(-2) + ':' +
	('00' + now.getMinutes()).slice(-2) + ':' +
	('00' + now.getSeconds()).slice(-2) + '.' +
	('000' + now.getMilliseconds()).slice(-3);
    logLines.push(timestamp + ' -> ' + s);
    logDisplay.value = logLines.join('\n');
    logDisplay.scrollTop = logDisplay.scrollHeight;
}


function msToTime(duration) {
    var milliseconds = Math.floor((duration % 1000) / 100),
	seconds = Math.floor((duration / 1000) % 60),
	minutes = Math.floor((duration / (1000 * 60)) % 60),
	hours = Math.floor((duration / (1000 * 60 * 60)) % 24);
    
    hours = (hours < 10) ? "0" + hours : hours;
    minutes = (minutes < 10) ? "0" + minutes : minutes;
    seconds = (seconds < 10) ? "0" + seconds : seconds;
    
    return hours + ":" + minutes + ":" + seconds + "." + milliseconds;
}


/**
 * Updates UI elements after starting / stopping streaming.
 */
function updateStreamConnectionState() {
    document.getElementById('a_running').checked = bleInstance.a_running;
    document.getElementById('a_runningsince').value = msToTime(Number(bleInstance.a_runningsince));
//    document.getElementById('a_battery').value = bleInstance.a_battery.toFixed(2) + ' V';;
    const batteryElem = document.getElementById('a_battery');
    batteryElem.value = bleInstance.a_battery.toFixed(2) + ' V';

    // ✅ Add color-coding based on voltage
    const voltage = bleInstance.a_battery;
    if (voltage > 3.9) {
        batteryElem.style.color = 'green';
    } else if (voltage > 3.6) {
        batteryElem.style.color = 'orange';
    } else {
        batteryElem.style.color = 'red';
    }
    document.getElementById('a_firmware').value = bleInstance.a_firmware;
    document.getElementById('a_default_params').value = bleInstance.a_default_params;
    
    
    if(bleInstance.connected && bleInstance.a_running) {
	document.getElementById('toggleStream').innerHTML = 'Stop Stream';	      
	document.getElementById('s_volume').disabled = true;
	document.getElementById('s_volume_text').disabled = true;
	document.getElementById('s_chan8').disabled = true;
	document.getElementById('s_stimfreq').disabled = true;
	document.getElementById('s_stimdur').disabled = true;
	document.getElementById('s_cycleperiod').disabled = true;
	document.getElementById('s_pauzecycleperiod').disabled = true;
	document.getElementById('s_pauzedcycles').disabled = true;
	document.getElementById('s_jitter').disabled = true;
	document.getElementById('s_test_mode').disabled = true;	      
    } else {
	document.getElementById('toggleStream').innerHTML = 'Start Stream';
	document.getElementById('s_volume').disabled = false;
	document.getElementById('s_volume_text').disabled = false;
	document.getElementById('s_chan8').disabled = false;
	document.getElementById('s_stimfreq').disabled = false;
	document.getElementById('s_stimdur').disabled = false;
	document.getElementById('s_cycleperiod').disabled = false;
	document.getElementById('s_pauzecycleperiod').disabled = false;
	document.getElementById('s_pauzedcycles').disabled = false;
	document.getElementById('s_jitter').disabled = false;
	document.getElementById('s_test_mode').disabled = false;	      
    }
}


/**
 * Updates UI elements after connecting or disconnecting BLE.
 * @param {boolean} connected A flag to indicate whether a BLE device
 *    is currently connected.
 */
function updateBLEConnectionState() {
    if (bleInstance.connected) {
	document.getElementById('clientConnectButton').innerHTML = 'Disconnect';
	document.getElementById('toggleStream').disabled = false;
	document.getElementById('refreshButton').disabled = false;	      
    } else {
	document.getElementById('clientConnectButton').innerHTML = 'Connect';
	document.getElementById('toggleStream').disabled = true;
	document.getElementById('refreshButton').disabled = true;
    }

    updateStreamConnectionState();

    if (bleInstance.connected) {
/** 	
    if (bleInstance.requestFirmwareVersion) {
   bleInstance.requestFirmwareVersion();
    }
*/
	fetch('presets.json')
	    .then(response => response.json())
	    .then(data => {
		const presetSelect = document.getElementById('presetSelect');
		presetSelect.length = 0;
		
		// Add a disabled dummy option as the first entry
		const dummyOption = document.createElement('option');
		dummyOption.text = "Select an option";
		dummyOption.value = "";
		dummyOption.disabled = true;
		dummyOption.selected = true; 
		presetSelect.add(dummyOption);
		
		data.presets.forEach(preset => {
		    const option = document.createElement('option');
		    option.value = preset.name;
		    option.text = preset.name;
		    option.dataset.values = JSON.stringify(preset);
		    presetSelect.add(option);
		});

		// Check URL for preset parameter and apply if present
		const presetParam = getUrlParameter('preset');
		if (presetParam) {
		    const preset = data.presets.find(p => p.name === presetParam);
		    if (preset) {
			slowApplyPresetValues(preset);
			presetSelect.value = presetParam; // Set the select to the current preset
		    } else {
			console.error(`Preset "${presetParam}" not found.`);
			alert(`Preset "${presetParam}" not found.`);
		    }
		}
		
	    })
	    .catch(error => console.error('Error loading presets:', error));

    }
}


/**
 * On receive settings batch from BLE, parse and update UI elements.
 */
function updateFromSettingsBatch() {
    document.getElementById('s_chan8').checked = bleInstance.s_chan8;
    document.getElementById('s_stimfreq').value = bleInstance.s_stimfreq;
    document.getElementById('s_stimdur').value = bleInstance.s_stimduration;
    document.getElementById('s_cycleperiod').value = bleInstance.s_cycleperiod;
    document.getElementById('s_pauzecycleperiod').value = bleInstance.s_pauzecycleperiod;
    document.getElementById('s_pauzedcycles').value = bleInstance.s_pauzedcycles;
    document.getElementById('s_jitter').value = bleInstance.s_jitter;
    document.getElementById('s_single_channel').value = bleInstance.s_single_channel;
    document.getElementById('s_test_mode').checked = bleInstance.s_testmode;
    document.getElementById('s_single_channel').disabled = !bleInstance.s_testmode;
}

/**
 * Update UI elements when volume reading is received from BLE.
 */
function updateVolume(num) {
    logFunction("Setting volume to: " + num);
    document.getElementById("s_volume").value = num;
    document.getElementById("s_volume_text").value = num;
}

function user_updatevolume(num) {
    logFunction("Update volume to: " + num);    
    //document.getElementById("s_volume").value = num;
    document.getElementById("s_volume_text").value = num;
    bleInstance.setVolume(num);
}


// function user_updatevolume2(num) {
//     document.getElementById("s_volume").value = num;
//     bleInstance.setVolume(num);
// }


document.getElementById('s_test_mode').addEventListener('change', function() {
    document.getElementById('s_single_channel').disabled = !this.checked;
  });


document.addEventListener("DOMContentLoaded", function() {
          // Make sure all sections are hidden initially
          document.querySelectorAll('.hidden-section').forEach(section => {
              section.style.display = 'none';
          });
	  
          function toggleSection(sectionId) {
              const section = document.getElementById(sectionId);
              if (section.style.display === 'none' || section.style.display === '') {
                  section.style.display = 'block';
              } else {
                  section.style.display = 'none';
              }
          }
	  
          window.toggleSection = toggleSection; // Expose toggleSection to global scope
      });


// Initialize
var bleInstance = new BleManager(logFunction,
				 updateBLEConnectionState,
				 updateVolume,
				 updateStreamConnectionState,
				 updateFromSettingsBatch);





loadPresets();
