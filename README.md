# PC Command Controlled LED and Alarm System using STM32

## Overview
This project is a real-time embedded system developed using the STM32 Nucleo board and PlatformIO. The system allows a user to control LED and buzzer operations through serial commands sent from a PC terminal.

The project demonstrates UART communication, GPIO interfacing, embedded programming, and real-time control using STM32 microcontrollers.

---

## Features

- LED ON/OFF control
- Normal blink mode
- Fast blink mode
- Slow blink mode
- SOS emergency pattern
- Alarm mode with buzzer
- Real-time UART command control
- Serial terminal interaction

---

## Hardware Components

- STM32 Nucleo Board
- Breadboard
- LED
- 330Ω Resistor
- Active Buzzer
- Jumper Wires
- USB Cable

---

## Software Used

- Visual Studio Code
- PlatformIO
- STM32Cube Framework
- GitHub

---

## UART Commands

| Command | Function |
|----------|-----------|
| 1 | LED ON |
| 2 | LED OFF |
| 3 | Normal Blink |
| 4 | Fast Blink |
| 5 | Slow Blink |
| 6 | SOS Pattern |
| 7 | Alarm Mode |

---
## Project Demonstration Video

[Watch Project Demo on YouTube] 

((https://youtu.be/hGDSHmoei50?si=qCnwx_R03K_R1SXU))

## Circuit Functionality

The STM32 receives commands from the PC using UART serial communication at 9600 baud rate. Based on the received command, the microcontroller changes the LED and buzzer behavior in real time.

---

## Project Structure

```text
src/main.c
platformio.ini
README.md
