# Simulator Intelligent Electronic Device (IED)
This project is an interface that provides basic functionality found in Industrial Control Systems devices, such as Intelligent Electronic Devices (IED). The goal of this project is to provide a lab device that will help students understand hardware analysis and exploitation techniques. The base device is a [ARM® Cortex®-M4F Based MCU TM4C123G LaunchPad™ Evaluation Kit](http://www.ti.com/tool/EK-TM4C123GXL) which provides multiple capabilities to interface with embedded device components such as microcontollers, EEPROM, fieldbus protocols, serial protocols, firmware, and memory.

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
