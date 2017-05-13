#include <xc.h>
#include <stdio.h>
#include "main.h"
#include "configBits.h"
#include "constants.h"
#include "lcd.h"
#include "I2C.h"
#include "macros.h"
#include "eeprom_routines.h"
#include "pwm.h"

const char keys[] = "123A456B789C*0#D";         //function and keypad declarations
void set_time(void);
void start(void);
void standby(void);
void info1(void);
void info2(void);
void info3(void);
void logs(void);
void load(unsigned char);
void timerTest();
  //EEPROM initialization (Fills EEPROM with four preset logs
__EEPROM_DATA(0x08,0x38,0x00,0x00,0x00,0x00,0x00,0x00);//Pointer variables (oldest, newest)

__EEPROM_DATA(0x00,0x32,0x11,0x00,0x26,0x11,0x16,0x00);
__EEPROM_DATA(0x02,0x22,0x12,0x05,0x00,04,03,00); //Oldest run 

__EEPROM_DATA(00,0x21,0x17,00,0x11,0x12,0x16,00);
__EEPROM_DATA(01,0x56,0x12,04,01,02,05,00);

__EEPROM_DATA(00,0x57,0x22,00,0x13,01,0x17,00);
__EEPROM_DATA(00,0x59,0x12,06,02,01,03,00);

__EEPROM_DATA(00,0x46,0x12,00,0x31,01,0x17,00); //Latest run
__EEPROM_DATA(01,0x43,0x12,02,01,04,05,00);

const static unsigned char EEPROM_CAP = 0x38;       //EEPROM CAP AND FLOOR
const static unsigned char EEPROM_FLOOR = 0x8;
volatile static unsigned char EEPROM_OLD;
volatile static unsigned char EEPROM_NEW;

struct sortData {
    unsigned char date_time[7];
    unsigned char totTime[2];
    unsigned char totCans;
    unsigned char lbl;
    unsigned char no_lbl;
    unsigned char tab;
    unsigned char no_tab;
};
volatile static struct sortData data;               //sorting log struct definition

//EEPROM Store function, called after successful run to store permanent log
void store() {
    EEPROM_OLD = EEPROM_READ(0x00);                 //find the oldest log
    for(unsigned char i=0;i<0x07;i++) {             
        EEPROM_WRITE(EEPROM_OLD + i,data.date_time[i]);  
    }
    EEPROM_WRITE(EEPROM_OLD + 0x08,data.totTime[0]);
    EEPROM_WRITE(EEPROM_OLD + 0x09,data.totTime[1]);
    EEPROM_WRITE(EEPROM_OLD + 0x0A,data.totCans);
    EEPROM_WRITE(EEPROM_OLD + 0x0B,data.lbl);
    EEPROM_WRITE(EEPROM_OLD + 0x0C,data.no_lbl);
    EEPROM_WRITE(EEPROM_OLD + 0x0D,data.tab);
    EEPROM_WRITE(EEPROM_OLD + 0x0E,data.no_tab);    //Store sorting data, replacing oldest log
    EEPROM_NEW = EEPROM_OLD;
    if(EEPROM_OLD == EEPROM_CAP) {
        EEPROM_OLD = EEPROM_FLOOR;
    } else {
        EEPROM_OLD = EEPROM_OLD + 0x10;
    }
    EEPROM_WRITE(0x00,EEPROM_OLD);
    EEPROM_WRITE(0x01,EEPROM_NEW);                  //Update old and new log pointers
    return;
}
void  standby(void) {
    initLCD();
    unsigned char time[7];


    I2C_Master_Init(10000);                         //Initialize I2C Master for RTC
    MOTORS = 0;
    PWM1_Stop();
    PWM2_Stop();
    
    //set_time();
    while(1) {
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '1' || temp == '2' || temp == '9') {
                while(PORTBbits.RB1 == 1);
                if(temp == '1') {
                    start();
                } else if(temp == '2') {
                    logs();
                } else if(temp == '9') {
                    picToPC();    
                } else if(temp == '5') {
                    set_time();
                }
            }
        }
        //Reset RTC memory pointer 
        I2C_Master_Start(); //Start condition
        I2C_Master_Write(0b11010000); //7 bit RTC address + Write
        I2C_Master_Write(0x00); //Set memory pointer to seconds
        I2C_Master_Stop(); //Stop condition

        //Read Current Time
        I2C_Master_Start();
        I2C_Master_Write(0b11010001); //7 bit RTC address + Read
        for(unsigned char i=0;i<0x06;i++){
            time[i] = I2C_Master_Read(1);
        }
        time[6] = I2C_Master_Read(0);       //Final Read without ack
        I2C_Master_Stop();
        printf("%02x/%02x/%02x", time[6],time[5],time[4]);    //Print date in YY/MM/DD
        
        printf("   %02x:%02x", time[2],time[1]);    //HH:MM:SS
        __lcd_newline();
        RS = 0;
        printf("1-START   2-LOGS");
        lcdInst(0b10000000);
        __delay_ms(10);
        
    }
    return;

}


void start(void) {
    unsigned int val1, val2, val3, testval3, flexval;
    unsigned char loc1, loc2;
    val1 = 0;
    val2 = 15;
    no_lbl = 0;
    no_tab = 0;
    tab = 0;
    lbl = 0;
    threshcalibrate = 0;
    sumflex = 0;
    sumflexcount = 0;
    idlecount = 0;
    ei();
    flag1 = 0;
    flag2 = 0;
    
    //TRISC = 0x11110001;
    s1state = MIDSERVO;
    s2state = MIDSERVO;
    
    //__lcd_clear();
    //__delay_ms(10);
    
    unsigned char time[7];
    elapsedTime[0] = 0;
    elapsedTime[1] = 0;
    secTime = 0;
    //<editor-fold defaultstate="collapsed" desc="Set Timer0">
    T0CON = 0b00010111;
    TMR0H = 0b10000101;
    TMR0L = 0b11101110;
    T0CON = T0CON | 0b10000000; //set 1s increment timer interrupts
    //</editor-fold>
    
    RS = 0;
    printf("RUNNING         ");
    __lcd_newline();
    RS = 0;
    printf("%02d:%02d           ",elapsedTime[0],elapsedTime[1]);
    __lcd_home();
    sort1=0;count1=0;sort2=0;count2=0;

    MOTORS = 1;
    PROXPOW = 1;
    FLEXPOW = 1;
    CONDPOW = 1;
    opRun = 1;
    
    
    // <editor-fold defaultstate="collapsed" desc="Set Timer1">

    T1CON = 0b10110000;
    TMR1H = 0b10110001;
    TMR1L = 0b11100000;
    TMR1ON = 1;
    
    // </editor-fold>    //20ms
    __delay_ms(10);
    // <editor-fold defaultstate="collapsed" desc="Set Timer3">
    T3CON = 0b10110000;
    TMR3H = 0b10110001;
    TMR3L = 0b11100000;
    T3CON = T3CON | 0b00000001;
    // </editor-fold>   //20ms
    
                // Initialize timer again!
    T2CON = 0b00111001; // 
    PR2 = 0b01111101;
    T2CON = T2CON | 0b00000100;
    //flagx = 0, no can in channel x being sorted
    //flagx = 1, can x being moved to bay and tested
    //flagx = 2, can x finished testing, being moved to final location A
    //flagx = 3, can x finished testing, being moved to final location B
    while(opRun) {
        if(testing == 1) {
            __lcd_home();
            printf("flag1:%d  flag2:%d",flag1,flag2);
            __lcd_newline();
            RS = 0;
            printf("l:%dnl:%d  t:%dnt:%d",lbl,no_lbl,tab,no_tab);
        } else if(testing == 2) {
            __lcd_home();
            unsigned int testval1 = ADCread(0);
            if(testval1 > flexthresh2) {
                testval3 = testval1;
            } else if(testval1 < flexthresh1) {
                testval3 = testval1;
            }
            printf("FLEX:%d %d %d",testval1,testval3,sumflex);
            __lcd_newline();
            RS = 0;
            printf("1:%d 2:%d %d",flexthresh1,flexthresh2,sumflexcount);
        } else if(testing == 3) {
            __lcd_home();
            printf("PROX1: %d        ",PROX1);
            __lcd_newline();
            RS = 0;
            printf("PROX2: %d        ",PROX2);
        } else if(testing == 4) {
            __lcd_home();
            printf("S1: %d           ",s1state);
            __lcd_newline();
            RS = 0;
            printf("S2: %d           ",s2state);
        }
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '0') {
                while(PORTBbits.RB1 == 1);
                testing++;
                if(testing == 5) {
                    testing = 0;
                }  
            }
        }
        if(flag1 == 1) {
            if(read1) {
                if(CONDIN == 1) {
                    flag1 = 2;
                }
                read1 = 0;
            } 
        }
        if(flag2 == 1) {
            if(read2) {
                val2 = ADCread(0);
                if(val2<flexthresh1 || val2>flexthresh2) {
                    flag2 = 2;
                }
                read2 = 0;
            }
        }
        if(threshcalibrate == 1) {
            flexval = ADCread(0);           //read flex sensor value                          
            di();
            sumflex = sumflex + flexval;    //add reading to sum
            sumflexcount++;                 //increment num of readings taken
            ei();
            threshcalibrate = 0;            //calibrate flag update
        }
        if(sumflexcount > MAXCALIBCOUNT) {  //Ten readings have been taken
            avgflex = sumflex / sumflexcount;
            flexthresh1 = avgflex - 15;
            flexthresh2 = avgflex + 15;     //Set flex thresholds
            sumflex = 0;
            sumflexcount = 0;               //reset sum and reading count
            threshcalibrate = 0;            //calibrate flag update
        }
    }
    di();
    MOTORS = 0;
    FLEXPOW = 0;
    CONDPOW = 0;
    PROXPOW = 0;
    SOL1 = 0;
    SOL2 = 0;
    __lcd_home();
    PWM1_Stop();
    PWM2_Stop();
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition
    
    //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read
    for(unsigned char i=0;i<0x06;i++){
        data.date_time[i] = I2C_Master_Read(1);
    }
    data.date_time[6] = I2C_Master_Read(0);       //Final Read without ack
    I2C_Master_Stop();
    data.lbl = lbl;
    data.totCans = lbl + no_lbl + no_tab + tab;
    data.no_lbl = no_lbl;
    data.tab = tab;
    data.no_tab = no_tab;
    data.totTime[0] = elapsedTime[0];
    data.totTime[1] = elapsedTime[1];
    store();
    
    di();
    while(1) {
        printf("SORTING COMPLETE");
        __lcd_newline();
        RS = 0;
        printf("1-STANDBY 2-INFO");
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '1' || temp == '2') {
                while(PORTBbits.RB1 == 1);
                if(temp == '1') {
                    asm("goto _standby");
                } else if(temp == '2') {
                    asm("goto _info1");
                }
            }
        }
        __delay_ms(50);
    }
    return;   
}


void main(void) {
    startupConfig();

    //timerTest();

    EEPROM_OLD = EEPROM_READ(0x00);
    EEPROM_NEW = EEPROM_READ(0x01);
    standby();
    return;
}

void info1() {
    __lcd_home();
    printf("TOTAL TIME: %01d:%02d",data.totTime[0],data.totTime[1]);
    __lcd_newline();
    RS = 0;
    printf("CANS:%02d   2-NEXT",data.totCans);
    while(1) {
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '2') {
                while(PORTBbits.RB1 == 1);
                asm("pagesel _info2");
                asm("goto _info2");
            }
        }    
        __delay_ms(20);
    }
}
void info3(void) {
    __lcd_home();
    printf("LBL:%02x NO_LBL:%02x",data.lbl,data.no_lbl);
    __lcd_newline();
    RS = 0;
    printf("1-BACK 2-STANDBY");
    while(1) {
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '1'||temp == '2') {
                while(PORTBbits.RB1 == 1);
                if(temp == '1') {
                    asm("pagesel _info2");
                    asm("goto _info2");
                } else {
                    asm("pagesel _standby");
                    asm("goto _standby");;
                }
            }
        }
        __delay_ms(20);
    }
}

void info2(void) {
    __lcd_home();
    printf("TAB:%02d NO_TAB:%02d",data.tab,data.no_tab);
    __lcd_newline();
    RS = 0;
    printf("1-BACK    2-NEXT");
    
    while(1) {
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '1'||temp == '2') {
                while(PORTBbits.RB1 == 1);
                if(temp == '1') {
                    asm("pagesel _info1");
                    asm("goto _info1");
                } else {
                    asm("pagesel _info3");
                    asm("call _info3");
                }
            }
        }
        __delay_ms(20);
    }   
}

void logs(void) {
    unsigned char page = 1;
    unsigned char address = EEPROM_READ(0x01);
    while(page < 4) {
        for(unsigned char i=0;i<0x07;i++) {
            data.date_time[i] = EEPROM_READ(address + i);
        }
        __lcd_home();
        //printf("1-%02x/%02x/%02x %02x:%02x",data.date_time[6],data.date_time[5],
        //data.date_time[4],data.date_time[2],data.date_time[1]);
        printf("1-LOG %02x/%02x/%02x  ",data.date_time[6],data.date_time[5],data.date_time[4]);
        __lcd_newline();
        RS = 0;
        printf("2-STANDBY 3-NEXT");
        while(1) {
            if(PORTBbits.RB1 == 1) {
                unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
                unsigned char temp = keys[keypress];
                if(temp == '1'||temp == '2'||temp=='3') {
                    while(PORTBbits.RB1 == 1);
                    if(temp == '1') {
                        __lcd_home();
                        printf("%02x/%02x/%02x   %02x:%02x",data.date_time[6],data.date_time[5],data.date_time[4],data.date_time[2],data.date_time[1]);
                        __lcd_newline();
                        RS = 0;
                        printf("2-NEXT          ");
                        while(1) {
                            if(PORTBbits.RB1 == 1) {
                            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
                            unsigned char temp = keys[keypress];
                            if(temp == '2') {
                                while(PORTBbits.RB1 == 1);
                                load(address);
                                }
                            }    
                            __delay_ms(20);
                        }
                    } else if (temp=='2'){
                        asm("pagesel _standby");
                        asm("call _standby");
                    } else {
                        if(address == EEPROM_FLOOR) {
                           address = EEPROM_CAP;
                        } else {
                            address =  address - 0x10;
                        }
                        page++;
                        break;
                    }
                }
            }
        __delay_ms(20);
        }
    }
    __lcd_clear();
    __delay_ms(20);
    __lcd_home();
    for(unsigned char i=0;i<0x07;i++) {
    data.date_time[i] = EEPROM_READ(address + i);
    }
    printf("1-LOG %02x/%02x/%02x  ",data.date_time[6],data.date_time[5],data.date_time[4]);
    __lcd_newline();
    RS = 0;
    printf("2-STANDBY");
    while(1) {
        if(PORTBbits.RB1 == 1) {
            unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
            unsigned char temp = keys[keypress];
            if(temp == '2' || temp == '1') {
                while(PORTBbits.RB1 == 1);
                if(temp == '1') {
                    __lcd_home();
                    printf("%02x/%02x/%02x   %02x:%02x",data.date_time[6],data.date_time[5],data.date_time[4],data.date_time[2],data.date_time[1]);
                    __lcd_newline();
                    RS = 0;
                    printf("2-NEXT          ");
                    while(1) {
                        if(PORTBbits.RB1 == 1) {
                        unsigned char keypress = (PORTB & 0xF0)>>4; // Read the 4 bit character code)
                        unsigned char temp = keys[keypress];
                        if(temp == '2') {
                            while(PORTBbits.RB1 == 1);
                            load(address);
                            }
                        }    
                        __delay_ms(20);
                    }
                } else {
                    asm("pagesel _standby");
                    asm("goto _standby");
                }
            }
        }
        __delay_ms(20);
    }
}
      
void load(unsigned char address) {
    data.totTime[0] = EEPROM_READ(address + 0x08);
    data.totTime[1] = EEPROM_READ(address + 0x09);
    data.totCans = EEPROM_READ(address + 0x0A);
    data.lbl = EEPROM_READ(address + 0x0B);
    data.no_lbl = EEPROM_READ(address + 0x0C);
    data.tab = EEPROM_READ(address + 0x0D);
    data.no_tab = EEPROM_READ(address + 0x0E);
    asm("pagesel _info1");
    asm("goto _info1");
} 


const char initializationTime[7] = {  0x00, //00 Seconds 
                            0x03, //50 Minutes
                            0x13, //11th hour
                            0x04, //Wednesday 
                            0x12, //29th
                            0x04, //March
                            0x17};//2017
void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<7; i++){
        I2C_Master_Write(initializationTime[i]);
    }    
    I2C_Master_Stop(); //Stop condition
}