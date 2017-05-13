#ifndef CONSTANTS_H
#define	CONSTANTS_H         //Prevent multiple inclusion 

//LCD Control Registers
#define RS          LATDbits.LATD2          
#define E           LATDbits.LATD3
#define	LCD_PORT    LATD   
#define LCD_DELAY   30
#define MOTORS      LATAbits.LATA4
#define PROX1       PORTAbits.RA2
#define PROX2       PORTAbits.RA3
#define SOL1        LATAbits.LATA5
#define SOL2        LATEbits.LATE0
#define MAXCALIBCOUNT  10
#define LOWSERVO    2
#define MIDSERVO    3
#define HIGHSERVO   4
#define PROXPOW     LATCbits.LATC5
#define FLEXPOW     LATEbits.LATE2
#define CONDPOW     LATCbits.LATC7
#define CONDIN      PORTEbits.RE1
#define SERVO1      LATCbits.LATC1
#define SERVO2      LATCbits.LATC2
#endif	/* CONSTANTS_H */

