#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000

// --- Shutdown Flag ---
volatile unsigned char shutdown_flag = 0;

// --- LCD Function Prototypes ---
void lcd_init();
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_string(const char *str);

// --- Interrupt Service Routine ---
void __interrupt() ISR() {

    if(INTCONbits.INT0IF) {

        shutdown_flag = 1;

        INTCONbits.INT0IF = 0;
    }
}

// --- I2C Master Drivers ---
void I2C_Init() {
    TRISC3 = 1; 
    TRISC4 = 1;
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;
    SSPADD = 49;
}

void I2C_Wait() {
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
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

    I2C_Wait();

    RCEN = 1;

    I2C_Wait();

    unsigned char temp = SSPBUF;

    I2C_Wait();

    ACKDT = (ack)?0:1;

    ACKEN = 1;

    return temp;
}

// --- EEPROM Functions ---
void EEPROM_Log(unsigned char val) {

    I2C_Start();

    I2C_Write(0xA0);

    I2C_Write(0x01);

    I2C_Write(val);

    I2C_Stop();

    __delay_ms(10);
}

unsigned char EEPROM_Get() {

    I2C_Start();

    I2C_Write(0xA0);

    I2C_Write(0x01);

    I2C_Start();

    I2C_Write(0xA1);

    unsigned char val = I2C_Read(0);

    I2C_Stop();

    return val;
}

// --- Peripheral Drivers ---
void UART_Init() {

    TXSTAbits.TXEN = 1;

    RCSTAbits.SPEN = 1;

    RCSTAbits.CREN = 1;

    SPBRG = 31;
}

void UART_Send(const char *str) {

    while(*str) {

        while(!TXSTAbits.TRMT);

        TXREG = *str++;
    }
}

void ADC_Init() {

    ADCON1 = 0x0D;

    ADCON2 = 0xA9;

    ADCON0 = 0x01;
}

int ADC_Read(int ch) {

    ADCON0 = (ADCON0 & 0xC3) | (ch << 2);

    __delay_us(10);

    GO = 1;

    while(GO);

    return ((ADRESH<<8)+ADRESL);
}

// --- LCD Functions ---
void lcd_init() {

    __delay_ms(15);

    lcd_command(0x38);

    lcd_command(0x0C);

    lcd_command(0x06);

    lcd_command(0x01);

    __delay_ms(2);
}

void lcd_command(unsigned char cmd) {

    PORTD = cmd;

    PORTEbits.RE0 = 0;

    PORTEbits.RE1 = 1;

    __delay_ms(1);

    PORTEbits.RE1 = 0;
}

void lcd_data(unsigned char data) {

    PORTD = data;

    PORTEbits.RE0 = 1;

    PORTEbits.RE1 = 1;

    __delay_ms(1);

    PORTEbits.RE1 = 0;
}

void lcd_string(const char *str) {

    while(*str)
        lcd_data(*str++);
}

// --- Main System Logic ---
void main() {

    I2C_Init();

    UART_Init();

    ADC_Init();

    // LCD Pins
    TRISD = 0x00;
    TRISE = 0x00;

    lcd_init();

    // RB0 Interrupt Pin
    TRISBbits.TRISB0 = 1;

    // Interrupt Configuration
    INTCONbits.INT0IE = 1;

    INTCONbits.INT0IF = 0;

    INTCON2bits.INTEDG0 = 0;

    INTCONbits.GIE = 1;

    TRISC0 = 0;
    TRISC1 = 0;
    TRISC2 = 0;

    unsigned char peak_temp = EEPROM_Get();

    char buffer[32];

    UART_Send("Industrial Logger Online\r\n");

    UART_Send("Commands: [L] List Logs, [C] Clear\r\n");

    while(1) {

        // --- Emergency Shutdown ---
        if(shutdown_flag) {

            CCPR1L = 0;

            CCP1CON = 0x00;

            lcd_command(0x01);

            lcd_command(0x80);

            lcd_string("SYSTEM STOPPED");

            while(1);
        }

        int temp = ADC_Read(0) / 2;

        // --- LCD Display ---
        lcd_command(0x80);

        sprintf(buffer,"Temp:%d C ",temp);

        lcd_string(buffer);

        lcd_command(0xC0);

        sprintf(buffer,"Peak:%d C ",peak_temp);

        lcd_string(buffer);

        // Logging Logic
        if(temp > peak_temp) {

            peak_temp = temp;

            EEPROM_Log(peak_temp);

            UART_Send("NEW PEAK RECORDED!\r\n");
        }

        // Simple CLI
        if(PIR1bits.RCIF) {

            char cmd = RCREG;

            if(cmd == 'L' || cmd == 'l') {

                sprintf(buffer, "Peak Temp: %d C\r\n", EEPROM_Get());

                UART_Send(buffer);
            }

            if(cmd == 'C' || cmd == 'c') {

                EEPROM_Log(0);

                peak_temp = 0;

                UART_Send("LOGS CLEARED\r\n");
            }
        }

        // Preventive Maintenance (Fan Control)
        if(temp > 40) {

            CCPR1L = 255;

            CCP1CON = 0x0C;

            T2CON = 0x04;
        }
        else {

            CCPR1L = 100;
        }

        __delay_ms(500);
    }
}