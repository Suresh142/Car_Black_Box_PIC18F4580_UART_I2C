# Car Black Box Using UART & I2C Communication (PIC18F4850)

## Overview
This project implements a **Car Black Box** using the **PIC18F4850 microcontroller** in **MPLAB IDE**.  
Black Boxes are typically used in transportation systems (e.g., airplanes, cars) to log and analyze events for post-accident investigations.  
This system continuously monitors and records critical driving parameters and events such as **over-speeding** for accident analysis.

## Objectives
- Monitor and log key events in real time.
- Store event data in non-volatile memory.
- Provide easy data retrieval via **UART**.
- Interface with sensors and peripherals via **I2C**.

## Hardware Requirements
- **PIC18F4850** Microcontroller
- EEPROM (I2C) for data logging
- GPS Module (optional, for location logging)
- UART Interface (PC or Serial Terminal)
- Power Supply (12V → 5V regulated)

## Software Requirements
- MPLAB IDE
- XC8 Compiler
- Serial Terminal software (e.g., Tera Term, PuTTY)

## Communication Protocols Used
- **UART** → For serial data transmission and retrieval of event logs.
- **I2C** → For interfacing with external EEPROM for event storage.
