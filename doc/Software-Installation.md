# VHP Vibro Glove Program

## Setup & Installation Guide

### 1. Download and install Arduino IDE

Begin by downloading and installing the [Arduino IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).

### 2. Add the Adafruit nRF52 board to Arduino

Go to Arduino Preferences. Add https://adafruit.github.io/arduino-board-index/package_adafruit_index.json to Additional Board Manager URLs. For additional details see [Adafruit installation guide.](https://learn.adafruit.com/introducing-the-adafruit-nrf52840-feather/arduino-bsp-setup)

![image](https://github.com/F2HEAL/VHP-Vibro-Glove2/assets/18469570/30eecce4-ccd9-45cf-ac77-4a387cdd99d9)

On the left sidebar of the Arduino IDE, select the Boards Manager tab.

Search for "Adafruit nRF52", and select install on the package.

Then, connect the board to the computer using a USB cable.

Hopefully, the board will now be available to select at near the top of the Arduino IDE.

If the board does not show, try removing the battery if there is one, make sure it is turned on, and hold the reset button while plugging it in.

### 3. Install Arduino CLI (optional)

Follow [the instructions](https://github.com/arduino/arduino-cli) to install Arduino CLI. It uses the same libraries as Arduino IDE.

### 4. Download this code

Select the green **"Code"** button on [this](https://github.com/F2HEAL/VHP-Vibro-Glove2) GitHub page, and select the Download ZIP option from the dropdown. <br>
Extract the folder, and open the .ino file using the Arduino IDE.

### 5. Upload the program to the board

Connect the VHP board to the PC with a micro USB B cable and switch the board ON with its POWER switch.

Select Board
"Adafruit Feather nRF52840 Express"
and COM Port
 
Simply press the upload button at the top of the Arduino IDE (the arrow).

The Arduino IDE will output information about the compilation and upload. <br>
If it is unsuccessful, try once more. If an error persists, it may require a closer look.

### Now you are finished!

We hope this program helps you on your journey.

Please create an "issue" in the Issues tab of this GitHub repository if you would like to see something added.
