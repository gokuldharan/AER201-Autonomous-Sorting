#include <xc.h>
#include <stdio.h>
#include "I2C.h"
#include "configBits.h"
#include <p18f4620.h>
#include "lcd.h"
#include "constants.h"
#include "macros.h"
#include "eeprom_routines.h"

const static unsigned char EEPROM_CAP = 0x38;       //EEPROM CAP AND FLOOR
const static unsigned char EEPROM_FLOOR = 0x8;


void picToPC(void) {
    di();
    int log = 1;
    char value = '1';
    SPBRG = 0b00110011;
    TXSTA = 00000000;
    RCSTA = 00000000;
    SPEN = 1;
    CREN = 1;
    TXEN = 1;
    BRGH = 0;
    BRG16 = 0;
    int i;
    
    unsigned char transmit = 1;
    unsigned char startaddress = EEPROM_READ(0x01);
    unsigned char logaddress = startaddress;
    unsigned char readaddress = logaddress;
    lcdInst(0b10000000);
    printf("TRANSMITTING    ");  
    __lcd_newline();
    RS = 0;
    printf("                  ");
   
    while(transmit == 1) {
        if(TRMT == 1) {
            TXREG = EEPROM_READ(readaddress);
        
            readaddress++;
            if(readaddress == logaddress + 0x0F) {
                if(logaddress == EEPROM_FLOOR) {
                    logaddress = EEPROM_CAP;
                } else {
                    logaddress =  logaddress - 0x10;
                }
                readaddress = logaddress;
                if(logaddress == startaddress) {
                transmit = 0;
                }
            }
        }
    }
}

