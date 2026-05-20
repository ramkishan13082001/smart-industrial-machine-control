Industrial Fault Logger

Advanced PIC18F4580 project featuring I2C Data Logging, Preventive Maintenance logic, and Technician UART CLI.

🏭 Industry Standard Embedded Engineering

In real industrial environments, embedded systems are not just reactive — they must also be persistent and fault-aware.

This project introduces:

I2C Communication
External EEPROM Memory
UART Command Line Interface
ADC Sensor Monitoring
PWM Fan Control
Interrupt-Based Emergency Shutdown

The system continuously monitors machine temperature and load conditions while permanently storing critical fault information inside external EEPROM memory.

✨ Features
💾 Non-Volatile Fault Logging

Uses external 24C04 EEPROM through I2C communication.

Stored parameters:

Peak Temperature
Overload/Fault Status
Maintenance Warnings

Data remains stored even after power failure.

🛠️ Technician UART CLI

A real industrial-style serial console.

Technicians can connect using:

Proteus Virtual Terminal
USB-UART Converter
Serial Monitor
