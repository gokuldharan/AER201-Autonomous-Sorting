#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "macros.h"
#include "I2C.h"
#include "lcd.h"
#include "main.h"

// Interrupt handler
void interrupt handler(void) {   
    di();
    if(TMR0IF){                 //1s timer
        TMR0IF = 0;
        secTime++;
        elapsedTime[1]++;
        if(elapsedTime[1] >= 60) {
            elapsedTime[0]++;
            elapsedTime[1] = 0;
        }
        secTime = 60*elapsedTime[0] + elapsedTime[1];
        if(testing == 0) {
            __lcd_home();
            printf("RUNNING         ");
            __lcd_newline();
            RS = 0;
            printf("%02d:%02d           ",elapsedTime[0],elapsedTime[1]);
        }
        if(secTime<179 && lbl + no_lbl + tab + no_tab < 12 && idlecount < 4500){
            //Reinitialize timer
            T0CON = 0b00010111;
            TMR0H = 0b10000101;
            TMR0L = 0b11101110;
            T0CON = T0CON | 0b10000000;
        } else {
            opRun = 0;
        }
        //if(PORTEbits.RE1 == 1) {
        //    reset = 1;
        //}
    }
    
    //** 20 ms TIMER THAT CALLS printRTC() using Timer1 **
    if(TMR1IF){
        TMR1IF = 0;
        TMR1ON = 0;
        if(flag1 == 3) {
            count1++;
            if(count1>50) {
                lbl++;
                s1state = MIDSERVO;
                count1 = 0;
                flag1 = 0;
            }
        }
        
        if(flag1 == 2) {
            count1++;
            if(count1 >= 40) {
                SOL1 = 0;
            }
            if(count1>65) {
                s1state = HIGHSERVO;
            }
            if(count1>115) {
                no_lbl++;
                flag1 = 0;
                count1 = 0;
                s1state = MIDSERVO;   
            }
        }
        if(flag1 == 1) {
            count1++;
            if(count1 <= 40 || count1 >=65) {  
                SOL1 = 0;
            } else {
                SOL1 = 1;
            }
            read1 = 0;
            if(count1 > 40 && count1 < 105) {
                read1 = 1;
            } else if(count1 > 105) {
                flag1 = 3;
                count1 = 0;
                s1state = LOWSERVO;
            }
        }            
        if(flag1 == 0) {
            count1++;
            if(PROX1 == 1) {
                count1 = 0;
                idlecount++;
            }
        }
        if(flag1 == 0 && count1 == 1) {
            flag1 = 1;
            idlecount = 0;
            CONDPOW = 1;
            count1 = 0;
        }
        if(opRun) {
            T1CON = 0b10110000;
            TMR1H = 0b10110001;
            TMR1L = 0b11100000;
            TMR1ON = 1;
        }
    }
    
    //SERVO Timer
    if(TMR2IF){
        TMR2IF = 0;
        TMR2ON = 0;
        servoCount1++;
        servoCount2++;                          //Increment counts
        if(servoCount1 >= s1state) {            //End of Duty Cycle, set signal low
            SERVO1 = 0;
            if(servoCount1 >= (40-s1state)) {   //End of one full cycle, set signal high to begin next duty cycle
                servoCount1 = 0;
                SERVO1 = 1;
            }
        } 
        
        if(servoCount2 >= s2state) {            //End of Duty Cycle, set signal low
            SERVO2 = 0;
            if(servoCount2 >= (40-s2state)) {   //End of one full cycle, reset signal
                SERVO2 = 1;
                servoCount2 = 0;
            }
        }

        if(opRun){
            // Initialize timer again!
            T2CON = 0b00111001; // 
            PR2 = 0b01111101;
            T2CON = T2CON | 0b00000100;
        }
    }
    
    //** 20 ms TIMER - CHANNEL 2
    if(TMR3IF){
        TMR3IF = 0;
        TMR3ON = 0;
        if(flag2 == 3) {
            count2++;
            if(count2>70) {
                s2state = (MIDSERVO);
                no_tab++;
                flag2 = 0;
                //flexthresh1 = FLEXTHRESH1;
                //flexthresh2 = FLEXTHRESH2;
                count2 = 0;
            }
        }
        
        if(flag2 == 2) {
            count2++;
            if(count2>15) {
                SOL2 = 0;
            }
            if(count2>65) {
                s2state = HIGHSERVO;
            }
            if(count2>135) {
                tab++;
                s2state = (MIDSERVO);
                count2 = 0;
                //flexthresh1 = FLEXTHRESH1;
                //flexthresh2 = FLEXTHRESH2;
                flag2 = 0;
            }
        }
        if(flag2 == 1) {
            count2++;
            if(count2 <= 15) {       //Release can for 15*0.02s = 0.3s
                SOL2 = 1;
            } else {
                SOL2 = 0;
            }
            read2 = 0;
            if(count2 > 5 && count2 < 35) {
                read2 = 1;
            } else if(count2 > 65) {
                flag2 = 3;
                //FLEXPOW = 0;
                count2 = 0;
                s2state = LOWSERVO;
            }
        }            
        if(flag2 == 0) {
            count2++;
            threshcalibrate = 1;
            if(PROX2 == 1) {        //No can detected, remain idle
                count2 = 0;
                idlecount++;
            }
            if(count2 == 100) {    //Positive reading for 100*0.02s=2.0s
                flag2 = 1;        //Update flag
                threshcalibrate = 0;
                idlecount = 0;
                count2 = 0;       //Reset time count
            }
        }
        
        if(opRun){
            // Initialize timer
            T3CON = 0b10110100;
            TMR3H = 0b10110001;
            TMR3L = 0b11100000;
            T3CON = T3CON | 0b00000001;
        }
    }
    ei();
}
