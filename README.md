# Simulator Intelligent Electronic Device (IED)
This project is an interface that provides basic functionality found in Industrial Control Systems devices, such as Intelligent Electronic Devices (IED). The goal of this project is to provide a lab device that will help students understand hardware analysis and exploitation techniques. The base device is a [ARM® Cortex®-M4F Based MCU TM4C123G LaunchPad™ Evaluation Kit](http://www.ti.com/tool/EK-TM4C123GXL) which provides multiple capabilities to interface with embedded device components such as microcontollers, EEPROM, fieldbus protocols, serial protocols, firmware, and memory.

# Builds

The most recent build can be found in the firmware directory.

# Capabilities

* Firmware Extraction and Analysis
  * Custom firmware that can be extracted using OpenOCD and the board's On-board In-Circuit Debug Interface (ICDI).
  * Encyption key analysis in firmware and SRAM.
* Encryption Functionality
  * Encryption / decryption of hex strings.
  * Get / Set encryption key.
* EEPROM Data
  * Programs and interacts with a Microchip 24LC08B EEPROM via Inter-Integrated Circuit (I2C).
  * Data is automatically programmed to the EEPROM when the device's interface starts.
  * Data currently includes a password, encryption key in plain text, encryption key in ASCII, and encryption key encoded Base64.

# Board Pinout to EEPROM
```
           ----------
      A0  | EEPROM    |Vcc----VBus
      A1  | Microchip |WP-----GND
      A2  | 24LC08B   |SCL----PD_0/SCL---10k (Brown / Black / Orange / Gold) Ohm Resistor ---3.3v
 GND--GND |           |SDA----PD_1/SDA---10k (Brown / Black / Orange / Gold) Ohm Resistor ---3.3v
           ----------
```

# Usage
## Flashing LaunchPad
TODO
## Connecting to SIM-IED
* Connect EEPROM to the LaunchPad using the pinout above.
* Connect to system's USB and make sure it connects to your VM, if necessary.
  * Type `lsusb` and look for an entry that is similar to the following:
    * `Bus 003 Device 007: ID 1cbe:00fd Luminary Micro Inc. In-Circuit Debug Interface`
* Connect to the SIM-IED serial interface
  * `screen /dev/ttyACM0 115200`
* Interace with interface
  * Once connected the user will need to hit enter to interact with the device.
    * Enter may return, `Bad Command!`
  * Type `help` to list commands
    * NOTE: Back space is currently not enabled on this interface.
  * Hit the reset button on the LaunchPad
    * This should return `Control Things IED Interface!`

# Compile Flags

The following compile options were copied out of the TI CCS build settings. This should help users configure TI CCS to build the project successfully.

`
-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="/Applications/ti/ccs901/ccs/tools/compiler/ti-cgt-arm_18.1.5.LTS/include" --include_path="/Users/<USER>/ti/tivaware_c_series_2_1_4_178" --include_path="/Users/<USER>/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c123gxl" --include_path="/Applications/ti/ccs901/ccs/tools/compiler/ti-cgt-arm_18.1.5.LTS/include" --include_path="/Users/<USER>/ti/tivaware_c_series_2_1_4_178" --include_path="/Users/<USER>/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c123gxl" --advice:power="all" --define=ccs="ccs" --define=UART_BUFFERED --define=TARGET_IS_TM4C123_RB1 --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi
`

# TODO

* Write-up flashing via OpenOCD and the build settings from Code Composer Studio as there are special considerations for Stack / Heap sizes.
* Fix decryption one-off error when displaying decrypted data back to user.
* Update UART to be able to handle backspace.
* Add encryption of strings rather than hex data.
* Add MODBUSRTU functionality. Note this might require moving to second USB interface instead of doing UART thru the ICDI.

# Resources
This project takes from several efforts:

* [ControlThings.io](https://www.controlthings.io) - tools and training to help the Industrial Control System (ICS) industry.
* [Tiny-AED-c](https://github.com/kokke/tiny-AES-c) - Small portable AES128/192/256 in C
* [TI Code Composer Studio](http://www.ti.com/tool/CCSTUDIO) - CSS Integrated Development Environment (IDE) and the example projects it provides.
* [TI Lauchpad EK-TM4C123GLX Pinout Diagram](https://energia.nu/pinmaps/img/EK-TM4C123GXL.jpg)

# Bill of Materials (BOM)
The following items are required for this project.

* [TI Launchpad EK-TM4C123GLX](http://www.ti.com/tool/ek-tm4c123gxl?keyMatch=ek-tm4c123gxl&tisearch=Search-EN-Everything)
* [Microchip 24LC08B EEPROM](https://www.mouser.com/ProductDetail/Microchip-Technology/24LC08B-SN?qs=YUl711QUJY3X%252BgVM9S8n3g%3D%3D)
* 2 - [10k Ohm Resisters](https://www.amazon.com/Projects-100EP51210K0-10k-Resistors-Pack/dp/B0185FIOTA)
* [Breadboard](https://www.sparkfun.com/products/12043)
* 6 - [Jumper Wires](https://www.sparkfun.com/products/12795)
* Optional: [TI Code Composer Studio](http://www.ti.com/tool/CCSTUDIO)
