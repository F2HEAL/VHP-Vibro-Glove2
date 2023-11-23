# Usage

## Settings

The default settings are defined in [Settings.hpp](../VHP-Vibro-Glove2/Settings.hpp). They can be modified to change the startup behaviour of the software. Please refer to the file for more details.

## WebUI

The software can be controlled with a browser supporting Bluetooth Web API.

After starting the device will activate its Bluetooth connection, and start accepting connections. Note that the Bluetooth interface will deactivate after 5mins of inactivity.

Open the files found in [WebUI folder](../webui) in a browser to use this feature. A recent version can be on [WebUI F2Heal](https://webui.f2heal.com).

Use the connect button to connect to a device.

### Browsers

#### Chrome

Currently only working with the Chrome Browser with the 'enable-experimental-web-platform-features' flag enabled.

Change this in your browser by going to: 

     chrome://flags/#enable-experimental-web-platform-features

#### BlueFy

On Apple devices the [BlueFy](https://apps.apple.com/us/app/bluefy-web-ble-browser/id1492822055) Web Browser can also be used

## Internals

The software will produce a waveform in a random sequence on the different channels according to the settings passed to **SStream**.

These parameters are defined and documented in [SStream.hpp](VHP-Vibro-Glove2/SStream.hpp) and illustrated below.

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/2a48a60a-b9d9-407f-b9bf-ead3eb6b7bac)

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/fc01761d-4320-4044-b50b-c7768205cf2b)

 The default settings in [VHP-Vibro-Glove2.ino](VHP-Vibro-Glove2/VHP-Vibro-Glove2.ino) provide with a balanced default settings:
 * 8 channels
 * Samplerate 46875Hz, long story, tied to SetUpsamplingFactor(1), only touch it if you know what you're doing ;-)
 * Stimulation frequency 250Hz

      * This means that one stimulation sine cycle takes  46875Hz / 250Hz = 187.5 samples
 * Stimulation duration 100ms
      * This means that one stimulation period takes 4687.5 samples

* Cycle period 1332ms. This leaves 1332ms / 8  = 166.5ms as a maximum for stimulation duration. With a stimulation duration of 100ms this leaves 166.5 - 100 = 66.5ms of silence before the next channel starts

* Pauze-cyle period 5 & Pauzed cycles 2 : For every 5 cycles 2 will be pauzed, total silence on all channels. So on 5 * 1332ms = 6660ms there will 2 * 1332ms = 2664ms of silence
* Jitter 23.5% : This is 23.5% of 1332ms / 8 or 39.1ms, so well below the 66.5ms of silence calculated above


**WARNING:** it is up to the user to ensure that the configured values make sense, such as that the Jitter is not higher than silence after the stimulation. The [settings.ods](settings.ods) spreadsheet can be used to verify your settings.
