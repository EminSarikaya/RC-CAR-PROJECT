# 🏎️ RC Car and Custom Controller System

This project is a remote-controlled car system that communicates with its own custom controller, developed using two STM32F103C8T6 microcontrollers and NRF24L01 modules. 

## 📝 Project Purpose and Features
* **Seamless Communication:** Fast and reliable RF communication between the car and the controller using NRF24L01 modules.
* **Custom Controller Design:** A fully custom-designed and programmed controller specific to this project, replacing standard commercial transmitters.
* **Modular Structure:** The car and controller codes are structured to be developed and updated independently of each other.

## 🛠️ Hardware Used
**Car (Receiver) Side:**
* STM32F103C8T6 (Blue Pill) Microcontroller
* NRF24L01 2.4GHz RF Module
* TB6612FNG Motor Driver
* 2x ULN2003A Bipolar Transistor 
* LM2596 Voltage Regulator
* 4x DC Motors
* 4x MG90S Servo Motors
* 2x VTC6 Lithium-Ion Batteries

**Controller (Transmitter) Side:**
* STM32F103C8T6 (Blue Pill) Microcontroller
* NRF24L01 2.4GHz RF Module
* 3x Joystick Modules
* 0.96" OLED Display
* 4x4 Keypad
* LM2596 Voltage Regulator

### System Block Diagrams
#### Controller Side Block Scheme
![Controller Part Block Scheme](Images/Controller%20Part%20Block%20Scheme.png)

#### RC Car Side Block Scheme
![RC Car Part Block Scheme](Images/RC%20Car%20Part%20Block%20Scheme.png)

## 🚀 Setup and Usage
1. Clone this repository to your local machine:
   `git clone https://github.com/KullaniciAdin/RC-CAR-PROJECT.git`
2. Open the `projeAraba` folder with STM32CubeIDE (or your preferred IDE) and flash it to the microcontroller on the car.
3. Open the `ProjeKumanda` folder and flash it to the controller's microcontroller.
4. Power up both systems; the RF connection will be established automatically.
