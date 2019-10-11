#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions
// ---------------RTC Variables ------------------------------
int RTC; //Holds the RTC instance
int HH,MM,SS;
int sysHours = 0;
int sysMins = 0;
int sysSecs = 0;
const char RTCAddr = 0x6f;
const char SEC = 0x00; // see register table in datasheet
const char MIN = 0x01;
const char HOUR = 0x02;
const char TIMEZONE = 2; // +02H00 (RSA)
//-------------Button Variables-----------------
int lastInterruptTime = 0;
int stopAlarmSwitch = 21;
int resetSwitch = 22;
int freqSwitch = 23;
int stopSwitch = 25;
int frequency =1;
//------FUNCTION DECLARATION--------------
void setup(void);
void initGPIO(void);
int main(void);
void getTime(void);
void stop(void);
void reset(void);
void stopAlarm(void);
void changeFrequency(void);
void getSysTime(void);
//------------MAIN------------------------
int main(){
    setup();
    getTime();
    
}
//--------FUNCTIONS -----------------------
void getTime(void){
    SS = wirintPiI2CReadReg8(RTC,0x00);
    MM = wirintPiI2CReadReg8(RTC,0x01);
    HH = wirintPiI2CReadReg8(RTC,0x02);
    HH = HH+TIMEZONE;
    printf("Time: %x:%x:%x\n",HH,MM,SS);
    
}
void getSysTime(void){
    int currentSeconds = (HH*60*60)+(MM*60)+(SS*60);
}
void changeFrequency(void){
    long interruptTime = millis();
        if (interruptTime - lastInterruptTime>200){
            if(frequency == 1){
                frequency =2;
            }else if (frequency ==2){
                frequency =5;
            }else{
                frequency =1;
            }

        }
    lastInterruptTime = interruptTime;
}

void initGPIO(void){
    wiringPiSetup(); //This is the default mode
    //set all buttons to INPUT mode
    pinMode(stopAlarmSwitch,INPUT); 
    pinMode(resetSwitch,INPUT);
    pinMode(freqSwitch,INPUT);
    pinMode(stopSwitch,INPUT);
    //set all buttons to PULL-UP resistors
    pullUpDnControl(stopAlarmSwitch,PUD_UP);
    pullUpDnControl(resetSwitch,PUD_UP);
    pullUpDnControl(freqSwitch,PUD_UP);
    pullUpDnControl(stopSwitch,PUD_UP);
    //Set up Button Interrupts
    wiringPiISR (stopAlarmSwitch, INT_EDGE_FALLING,stopAlarm);
    wiringPiISR (resetSwitch, INT_EDGE_FALLING, reset);
    wiringPiISR (freqSwitch, INT_EDGE_FALLING,changeFrequency);
    wiringPiISR (stopSwitch, INT_EDGE_FALLING, stop);

}

void setup(void){
    RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC
    printf("RTC Time\tSysTimer\tHumidity\tTemp\tLight\tDAC Out\tAlarm Out");
    // set up ADC, DAC, RTC here
}