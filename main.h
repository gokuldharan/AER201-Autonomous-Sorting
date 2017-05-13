/* 
 * File:   main.h
 * Author: Gokul
 *
 * Created on February 24, 2017, 10:34 PM
 */
void main(void);
void startupConfig(void);
int PWM_Max_Duty(void);
void set_PWM_freq(long fre);
void set_PWM1_duty(unsigned int duty);
void set_PWM2_duty(unsigned int duty);
void PWM1_Start(void);
void PWM2_Start(void);
void PWM1_Stop(void);
void PWM2_Stop(void);
unsigned int ADCread(unsigned char channel);
int count1=0;
int count2=0;
int opRun = 0;
unsigned char elapsedTime[2]= {0,0};
unsigned char servoStates[2] = {1,1};
unsigned char secTime = 0;
unsigned char reset = 0;
int channel = 0;
int sort1 = 0;
int sort2 = 0;
int flag1 = 0;
int flag2 = 0;
int read1 = 0;
int read2 = 0;
unsigned char no_lbl = 0;
unsigned char lbl = 0;
unsigned char no_tab = 0;
unsigned char tab = 0;
unsigned char servoCount1;
unsigned char servoCount2;
unsigned char s1state;
unsigned char s2state;
//unsigned char PROX1 = 0;
//unsigned char PROX2 = 0;
unsigned char testing = 0;
int ch2sort = 0;
int idlecount = 0;
unsigned int flexthresh1;
unsigned int flexthresh2;
unsigned char threshcalibrate;
unsigned int sumflex;
unsigned int avgflex;
unsigned int sumflexcount;

