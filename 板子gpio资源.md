This guide details the pinout for the ESP32 Cheap Yellow Display (ESP32-2432S028R). It clarifies which pins connect to the onboard hardware and identifies the available free pins. This information is crucial for properly configuring the onboard components and for connecting external devices, such as a GPS or other peripherals. If your project requires additional I/O ports, consider exploring our project that demonstrates integrating an I/O extender with the ESP32 Cheap Yellow Display (ESP32-2432S028R).


When working with a standalone TFT Touchscreen Display 2.8 inch with an ILI9341 or ST7789 driver, this guide can be a helpful starting point for your project and how to connect your display.

The ESP32 Cheap Yellow Display Board – CYD (ESP32-2432S028R)
The ESP32-2432S028R is one of several options available from Yellow Display Boards, a popular choice among makers and hobbyists. The specific model used in this guide features a 2.8-inch TFT display with touch functionality, but other sizes are also available online, including 4.3″, 5.0″, and 7.0″ variants. These boards have gained recognition within the maker community as “Cheap Yellow Display” or CYD develop board, because of their affordability and versatility.

These boards are particularly convenient because they seamlessly integrate a TFT display with touch capabilities
and a popular developer microcontroller board, eliminating the need for manual wiring or creating a PCB. They have
proven to be extremely useful in my own projects and testing—thanks to their display and processing power, as well
as access to additional IO pins, which allows adding extra hardware as needed.

Next is shown the ESP32-2432S028R version 3 develop board


Back side


Front side

ESP32-2432S028R features
Dimensions
Module size 50.0×86.0mm
Product weight: approximately 50g
Connections
Serial
USB micro
USB-C (only on the v3)
Power
Operating Voltage: 5V
Power consumption: approximately 115mA
Microcontroller ESP-WROOM-32
Dual-core MCU, integrated WI-FI and Bluetooth functions
Frequency can reach 240MHz
520KB SRAM, 448KB ROM, Flash size is 4MB
TFT display ILI9341(v1,v2) or ST7789(v3)
2.8-inch color screen, support 16 BIT RGB 65K color display, display rich colors
240X320 resolution
Backlight control circuit
Onboard peripherals
TF card interface for external storage
RGB LED
built-in LDR (light-dependent resistor)
Speaker interface
Extended IO
Where to buy?
There are several stores where you can buy the ESP32-2432S028R the most common is Aliexpress. (By using my link you support Kafkar)

Different version of the ESP32-2432S028R board
As of this writing, there are three known versions of the ESP32-2432S028R board. Each version has distinct differences. To prevent potential issues when running your application, it’s crucial to identify the specific board version you are using at the start of your project.

Here’s a breakdown of the modifications across the three versions:

Version 1:
Display Type: ILI9341 SPI
Flash Memory (U4): 25Q32JVS10
Version 2:
Display Type: ILI9341 SPI
Flash Memory (U4): 25Q32JVS10 removed
GPIO022 added to the CN1 header
Version 3:
Display Controller/Driver: Changed to ST7789 SPI, MODE 3
Features two USB ports (YD2USB)
The Central Processing Unit (CPU) of the ESP32-2432S028R Board
The ESP32-2432S028R board utilizes the ESP-WROOM-32 module as its central processing unit (CPU). This is a crucial detail, as all input/output (I/O) pins originate from this module. Consequently, the capabilities and limitations of the ESP-WROOM-32 directly define those of the ESP32-2432S028R board. Many pins on the ESP-WROOM-32 offer multiple functionalities, including General Purpose Input/Output (GPIO), Analog-to-Digital Conversion (ADC), and Universal Asynchronous Receiver/Transmitter (UART). This versatility is powerful, but it necessitates meticulous planning during development.

For detailed specifications, please refer to the ESP-WROOM-32 datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf

This is the chip configuration used on the ESP32-2432S028R Board.

CPU and On-Chip Memory
ESP32-D0WD-V3 or ESP32-D0WDR2-V3 embedded, Xtensa dual-core 32-bit LX6 microprocessor, up to 240 MHz
448 KB ROM
520 KB SRAM
16 KB SRAM in RTC
Wi-Fi
802.11b/g/n
Bit rate: 802.11n up to 150 Mbps
A-MPDU and A-MSDU aggregation
0.4 μs guard interval support
Center frequency range of operating channel: 2412 ~ 2484 MHz
Bluetooth®
• Bluetooth V4.2 BR/EDR and Bluetooth LE specification
• Class-1, class-2 and class-3 transmitter
• AFH
• CVSD and SBC
Peripherals
• Up to 26 GPIOs – 5 strapping GPIOs
• SD card, UART, SPI, SDIO, I2C, LED PWM, Motor PWM, I2S, IR, pulse counter, GPIO, capacitive touch sensor, ADC, DAC, TWAI® (compatible with ISO 11898-1, i.e. CAN Specification 2.0)
Integrated Components on Module
40 MHz crystal oscillator
4 MB SPI flash
Antenna Options
ESP32-WROOM-32E: On-board PCB antenna
Operating Conditions
Operating voltage/Power supply: 3.0 ~ 3.6 V
Operating ambient temperature:
– 85 °C version: –40 ~ 85 °C
– 105 °C version: –40 ~ 105 °C.
IO connectors of the ESP32-2432S028R Board
The ESP32-2432S028R Board has for connectors to connect extra hardware below is a list of connectors and connector type See above where these connectors are placed on the ESP32-2432S028R Board.


Connector	Type	Connecion
P1	4P 1.25mm JST	Serial
P3	4P 1.25mm JST	GPIO
P4	2P 1.25mm JST	Speaker
CN1	4P 1.25mm JST	GPIO (I2C)
IO connector pins of the ESP32-2432S028R Board
P3: Extended I/O Limitations
Connector P3 provides only a ground connection and lacks both a voltage input (Vin) and a 3.3V power supply. It is crucial to note that GPIO22 is shared with the CN1 connector, and GPIO21 is utilized by the backlight function. Consequently, only GPIO35 is available for unrestricted use.

Pin	Note
GND	
IO35	Input only pin, no internal pull-ups available
IO22	Also on the CN1 connector
IO21	Used for the TFT Backlight, so not really usable
CN1: Extended I/O
Connector CN1 provides a 3.3V power supply, a ground connection, and two general-purpose input/output (GPIO) pins available for user applications. It is important to note that GPIO22 is shared with the P3 connector.

Pin	Note
GND	
IO22	Shared with P3 connector
IO27	
3.3V	
P1: Power and UART0
The P1 connector provides both power and UART0 (Universal Asynchronous Receiver/Transmitter) connectivity. This connector facilitates powering the ESP32-2432S028R module and enabling serial communication. Notably, the serial communication pathway is also utilized by the CH340 converter, which manages the USB connections. Consequently, repurposing these pins for alternative functions is generally discouraged due to potential conflicts and complexities.

Pin	Note
VIN	
IO1	TX UART0
IO3	RX UART0
GND	
P4: Speaker Output
The P4 connector does not directly connect to an ESP32 GPIO pin. Instead, it interfaces with an SC8002B audio amplifier chip. This connector is specifically designed for connecting a speaker. The SC8002B amplifier is controlled via GPIO26 of the ESP32 to modulate audio output.

Pin	Note
V01	
V02	
ESP32 Pin Assignments on the ESP32-2432S028R Board
The ESP32-2432S028R board integrates a variety of peripherals that facilitate application. Key features include a 2.8-inch display with touch functionality and an SD card slot. As these peripherals are directly connected to the ESP32, understanding their pin assignments is essential for effective utilization. The following paragraphs detail the connection mapping for each peripheral, enabling seamless integration into your custom applications.

Display Pin Assignments: HSPI Communication
The TFT display utilizes the SPI (Serial Peripheral Interface) communication protocol, specifically HSPI, for data transfer with the ESP32. For a comprehensive guide on integrating and utilizing the display, please refer to: Mastering the cyd your ultimate-beginners guide to the cheap yellow display esp32 432s028r using platform-io

The following SPI pins are dedicated to the display functionality:

Display SPI Pin	GPIO
MISO (TFT_MISO)	GPIO 12
MOSI (TFT_MOSI)	GPIO 13
SCKL (TFT_SCLK)	GPIO 14
CS (TFT_CS)	GPIO 15
DC (TFT_DC)	GPIO 2
RST (TFT_RST)	
Backlight Pin	GPIO 21
Touchscreen Pin Assignments: SPI Communication
The touchscreen interface utilizes the SPI (Serial Peripheral Interface) communication protocol for data transfer with the ESP32. For a comprehensive guide on integrating and utilizing the touchscreen, please refer to: Mastering the cyd your ultimate beginners guide to the cheap yellow display esp32 2432s028r using platform io

The following SPI pins are dedicated to the touchscreen functionality:

Touchscreen SPI Pin	GPIO
IRQ (XPT2046_IRQ)	GPIO 36
MOSI (XPT2046_MOSI)	GPIO 32
MISO (XPT2046_MISO)	GPIO 39
CLK (XPT2046_CLK)	GPIO 25
CS (XPT2046_CS)	GPIO 33
MicroSD Card Pin Assignments: SPI Communication
The microSD card interface utilizes the SPI (Serial Peripheral Interface) communication protocol. Specifically, it employs the ESP32’s default VSPI pins for data transfer.

MicroSD card SPI	GPIO
MISO	GPIO 19
MOSI	GPIO 23
SCK	GPIO 18
CS	GPIO 5
RGB LED Pin Assignments:
The ESP32-2432S028R board features an integrated RGB LED on the rear side, which can be a valuable tool for debugging purposes. The following details outline the RGB LED pin assignments:

RGB LED	GPIO
Red LED	GPIO 4
Green LED	GPIO 16
Blue LED	GPIO 17
Light Dependent Resistor (LDR)
The ESP32-2432S028R board incorporates an ambient light sensor (LDR) located on the front side, adjacent to the display. This sensor can be utilized to dynamically adjust the display’s backlight brightness.

LDR	GPIO
LDR	GPIO 34
Expanding I/O Capabilities
For applications requiring additional I/O (Input/Output) pins beyond the board’s default configuration, modifications are necessary. Disconnecting connections from unused peripherals allows for repurposing those I/O pins to interface with custom external devices.

Alternatively, an I/O expander can be connected to the CN1 connector. However, it’s important to note that this method may not be compatible with all types of connections.