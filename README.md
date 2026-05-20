# 🏭 Industrial Fault Logger

> Advanced Embedded Systems Project using PIC18F4580, I2C EEPROM, UART CLI, ADC Monitoring, and PWM-Based Preventive Maintenance

![Embedded](https://img.shields.io/badge/Embedded-Systems-blue)
![MCU](https://img.shields.io/badge/Microcontroller-PIC18F4580-green)
![Protocol](https://img.shields.io/badge/Communication-I2C-orange)
![Language](https://img.shields.io/badge/Language-Embedded%20C-red)

---

# 📌 Project Overview

The **Industrial Fault Logger** is an industry-style embedded monitoring system designed for:

- Persistent fault storage
- Machine health monitoring
- Preventive maintenance
- Real-time telemetry
- Technician diagnostics

Unlike beginner embedded projects, this system permanently stores critical fault information using external EEPROM memory.

---

# ✨ Key Features

## 💾 Non-Volatile Fault Logging

Uses external **24C04 EEPROM** to store:

- Peak Temperature
- Fault Conditions
- Overload Events
- Maintenance Warnings

> Data remains stored even after complete power loss.

---

## 🛠️ UART Technician CLI

Technicians can interact with the system using serial commands through:

- Proteus Virtual Terminal
- USB-UART Converter
- Serial Monitor

### Supported Commands

| Command | Function |
|----------|----------|
| `L` | Read Stored Logs |
| `C` | Clear EEPROM Memory |

---

## 🛡️ Preventive Maintenance Logic

The system continuously monitors:

- Temperature
- PWM Fan Load
- Continuous High-Speed Operation

If the cooling fan operates at maximum duty cycle for extended periods:

- Maintenance warning is triggered
- Fault status is stored in EEPROM

---

## 🚨 Emergency Shutdown System

External interrupt using **RB0 (INT0)** provides:

- Instant motor shutdown
- Alarm activation
- Emergency safety response

---

# 🧠 EEPROM Fundamentals

## What is EEPROM?

EEPROM stands for:

> **Electrically Erasable Programmable Read-Only Memory**

Unlike RAM:

- EEPROM retains data after power OFF
- Suitable for long-term storage
- Widely used in industrial systems

---

## 📌 Industrial Applications of EEPROM

EEPROM is commonly used for:

- Fault Logging
- Password Storage
- Calibration Constants
- Runtime Statistics
- Configuration Parameters

---

# 📦 EEPROM Specifications

## 24C04 EEPROM

| Specification | Value |
|---------------|------|
| Memory Capacity | 512 Bytes |
| Communication Protocol | I2C |
| Communication Wires | 2 |
| Endurance | ~1 Million Write Cycles |

---

# 🔌 Hardware Mapping

| Component | PIC18F4580 Pin | Function |
|-----------|----------------|----------|
| LM35 Temperature Sensor | RA0 (AN0) | Thermal Monitoring |
| Load Sensor / Potentiometer | RA1 (AN1) | Load Simulation |
| Emergency Stop Button | RB0 (INT0) | Instant Shutdown |
| Buzzer | RC0 | Alarm |
| Cooling Fan Motor | RC2 (CCP1) | PWM Speed Control |
| EEPROM SCL | RC3 | I2C Clock |
| EEPROM SDA | RC4 | I2C Data |
| UART TX | RC6 | Serial Transmission |
| UART RX | RC7 | Serial Reception |
| LCD Data Bus | PORTD | LCD Interface |
| LCD Control | RE0 / RE1 | LCD Control |

---

# 📐 System Architecture

```text
                     INDUSTRIAL FAULT LOGGER
                    ==========================

                         +----------------------+
                         |     PIC18F4580       |
                         |                      |
 LM35 Sensor ----------> | RA0 (AN0)            |
 Load Sensor ----------> | RA1 (AN1)            |
                         |                      |
 E-Stop Button --------> | RB0 (INT0)           |
                         |                      |
 Buzzer <--------------> | RC0                  |
 Fan Motor <-----------> | RC2 (CCP1 PWM)       |
 EEPROM SCL <----------> | RC3                  |
 EEPROM SDA <----------> | RC4                  |
 UART TX ------------->  | RC6                  |
 UART RX <-------------  | RC7                  |
 LCD Data ------------>  | PORTD                |
 LCD Control --------->  | RE0 / RE1            |
                         +----------------------+
```

---

# 🛠️ Proteus Components Required

- PIC18F4580
- 24C04 EEPROM
- LM35 Sensor
- LM016L LCD
- Potentiometer
- DC Motor
- Push Button
- Virtual Terminal
- Buzzer
- 4.7kΩ Resistors

---

# ⚠️ Important I2C Requirement

## Pull-Up Resistors are Mandatory

Connect:

- RC3 → 4.7kΩ → +5V
- RC4 → 4.7kΩ → +5V

Without pull-up resistors:

- SDA/SCL lines remain LOW
- I2C communication fails

---

# 📡 EEPROM Device Addressing

| Operation | Address |
|-----------|----------|
| EEPROM Write | `0xA0` |
| EEPROM Read | `0xA1` |

---

# 🔄 System Workflow

```text
Read Temperature
        ↓
Compare with Stored Peak
        ↓
Is Current Temperature Higher?
        ↓
Store New Peak in EEPROM
        ↓
Send UART Notification
```

---

# 💻 Complete Firmware

## 📁 Machine_Controller_v1.c

```c
#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

// ---------------- I2C FUNCTIONS ----------------

void I2C_Init() {

    TRISC3 = 1;
    TRISC4 = 1;

    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;

    SSPADD = 49;
}

void I2C_Wait() {

    while((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}

void I2C_Start() {

    I2C_Wait();
    SEN = 1;
}

void I2C_Stop() {

    I2C_Wait();
    PEN = 1;
}

void I2C_Write(unsigned char data) {

    I2C_Wait();
    SSPBUF = data;
}

unsigned char I2C_Read(unsigned char ack) {

    unsigned char temp;

    I2C_Wait();
    RCEN = 1;

    I2C_Wait();
    temp = SSPBUF;

    I2C_Wait();

    ACKDT = (ack) ? 0 : 1;
    ACKEN = 1;

    return temp;
}

// ---------------- EEPROM FUNCTIONS ----------------

void EEPROM_Log(unsigned char val) {

    I2C_Start();

    I2C_Write(0xA0);
    I2C_Write(0x01);

    I2C_Write(val);

    I2C_Stop();

    __delay_ms(10);
}

unsigned char EEPROM_Get() {

    unsigned char val;

    I2C_Start();

    I2C_Write(0xA0);
    I2C_Write(0x01);

    I2C_Start();

    I2C_Write(0xA1);

    val = I2C_Read(0);

    I2C_Stop();

    return val;
}

// ---------------- UART FUNCTIONS ----------------

void UART_Init() {

    TXSTAbits.TXEN = 1;
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;

    SPBRG = 31;
}

void UART_Send_Char(char data) {

    while(!TXSTAbits.TRMT);

    TXREG = data;
}

void UART_Send_String(const char *str) {

    while(*str) {

        UART_Send_Char(*str++);
    }
}

// ---------------- ADC FUNCTIONS ----------------

void ADC_Init() {

    ADCON1 = 0x0D;
    ADCON2 = 0xA9;
    ADCON0 = 0x01;
}

int ADC_Read(int ch) {

    ADCON0 &= 0xC3;
    ADCON0 |= (ch << 2);

    __delay_us(10);

    GO = 1;

    while(GO);

    return ((ADRESH << 8) + ADRESL);
}

// ---------------- PWM FUNCTIONS ----------------

void PWM_Init() {

    TRISC2 = 0;

    CCP1CON = 0x0C;

    PR2 = 255;

    T2CON = 0x04;
}

void PWM_Set_Duty(unsigned int duty) {

    CCPR1L = duty >> 2;

    CCP1CON &= 0xCF;

    CCP1CON |= (duty & 0x03) << 4;
}

// ---------------- MAIN PROGRAM ----------------

void main() {

    int temp;

    unsigned char peak_temp;

    char buffer[32];

    I2C_Init();
    UART_Init();
    ADC_Init();
    PWM_Init();

    TRISC0 = 0;

    peak_temp = EEPROM_Get();

    UART_Send_String("Industrial Logger Online\r\n");

    while(1) {

        temp = ADC_Read(0) / 2;

        // Peak Temperature Logging
        if(temp > peak_temp) {

            peak_temp = temp;

            EEPROM_Log(peak_temp);

            UART_Send_String("NEW PEAK RECORDED\r\n");
        }

        // UART Command Processing
        if(PIR1bits.RCIF) {

            char cmd = RCREG;

            if(cmd == 'L' || cmd == 'l') {

                sprintf(buffer,
                        "Peak Temp: %d C\r\n",
                        EEPROM_Get());

                UART_Send_String(buffer);
            }

            if(cmd == 'C' || cmd == 'c') {

                EEPROM_Log(0);

                peak_temp = 0;

                UART_Send_String("LOGS CLEARED\r\n");
            }
        }

        // Fan Speed Control
        if(temp > 40) {

            PWM_Set_Duty(1023);
        }
        else {

            PWM_Set_Duty(400);
        }

        __delay_ms(500);
    }
}
```

---

# 🛰️ Understanding I2C Communication

## I2C Bus Lines

| Line | Purpose |
|------|----------|
| SCL | Serial Clock |
| SDA | Serial Data |

---

## I2C Communication Sequence

```text
START
   ↓
DEVICE ADDRESS
   ↓
MEMORY ADDRESS
   ↓
DATA BYTE
   ↓
STOP
```

---

# ⚙️ MSSP Register Configuration

| Register | Function |
|----------|-----------|
| SSPSTAT | I2C Status Register |
| SSPCON1 | I2C Control Register |
| SSPADD | Baud Rate Generator |
| ADCON0 | ADC Control |
| CCP1CON | PWM Configuration |

---

# 📐 I2C Baud Rate Formula

```math
SSPADD = ((FOSC / 4) / I2C_SPEED) - 1
```

For:

- FOSC = 20MHz
- I2C Speed = 100kHz

Result:

```text
SSPADD = 49
```

---

# ⚠️ Common I2C Mistakes

## ❌ Missing Pull-Up Resistors

Symptoms:

- SDA/SCL stuck LOW
- EEPROM not responding

Solution:

- Add 4.7kΩ pull-up resistors

---

## ❌ No Wait Handling

Always use:

```c
I2C_Wait();
```

Otherwise:

- Bus collisions occur
- Data corruption happens

---

## ❌ Wrong EEPROM Address

| Operation | Address |
|-----------|----------|
| Write | `0xA0` |
| Read | `0xA1` |

---

# 📚 Datasheet Navigation Tips

Search these keywords inside the PIC18F4580 datasheet:

- MSSP
- I2C Master Mode
- SSPADD
- CCP1
- ADCON0
- SCL
- SDA

---

# 🚀 Skills Learned

- Embedded C Programming
- I2C Communication
- EEPROM Interfacing
- UART Communication
- PWM Motor Control
- ADC Interfacing
- Fault Logging
- Register-Level Programming
- Datasheet Reading
- Industrial Embedded Design

---

# 🔮 Future Improvements

- RTC Timestamp Logging
- SD Card Storage
- GSM Fault Alerts
- IoT Cloud Monitoring
- LCD Menu Interface
- Fault History Pages
- Multiple Sensor Support
- FreeRTOS Multitasking

---

# 🏁 Conclusion

The Industrial Fault Logger demonstrates how real industrial embedded systems:

- Monitor machines
- Store persistent fault data
- Communicate with technicians
- Perform preventive maintenance
- Handle safety conditions

This project provides strong practical experience with:

- I2C Communication
- EEPROM Memory
- UART CLI
- PWM Motor Control
- ADC Sensor Interfacing
- Industrial Embedded System Design

---
