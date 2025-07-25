# IoT-PetFeeder.FinalProject

This project is a final semester project for the IoT course at my university, done in a group.

Our group project is an IoT Pet Feeder that uses ESP32 as the main microcontroller that integrates various sensors and actuators to provide an automated solution for feeding pets.

## Hardware Requirements
- ESP32 Development Board
- Servo Motor
- DHT11 Sensor Module
- Water Level Sensor Module
- Mini Water Pump
- LCD I2C 16x2
- Breadboard
- Jumper Wires
- Power Supply

## GPIO

| GPIO Pin | Usage | Usage Type |
| -------- | ----- | ---------- |
| GPIO 19 | DHT11 | Digital PWM Input + Output |
| GPIO 21 | I2C-SDA LCD | Digital PWM Input + Output |
| GPIO 22 | I2C-SCL LCD | Digital PWM Input + Output |
| GPIO 23 | Servo | Digital PWM Output |
| GPIO 32 | - | - |
| GPIO 33 | - | - |
| GPIO 34 | - | - |
| GPIO 35 | - | - |
| GPIO 36 | Water Level Sensor | Analog Input |
| GPIO 39 | - | - |

## Software Setup

1. Clone this repository
2. Install PlatformIO extension on Visual Studio Code
3. Open the project folder in Visual Studio Code, PlatformIO will automatically install the required libraries
4. Define and adjust configurations in `Hardware/include/config.h` if needed
5. Connect the ESP32 to your computer
6. Upload the code to the ESP32 using PlatformIO

## LICENSE

I license this project source code is under the usual [MIT License](LICENSE), however hardware designs and other assets made by other people on the project team may have different licenses.