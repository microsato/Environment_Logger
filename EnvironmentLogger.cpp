
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <mcp3004.h>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <pthread.h>
#include <stdlib.h> // For system functions
#include <unistd.h> //for timing functions
using namespace std;
// ---------------RTC Variables ------------------------------
int RTC; //Holds the RTC instance
int HH,MM,SS;
int SPI_CHAN=0;
int BASE=100;
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
int buzzer = 7;
int lastAlarmTime=0;
int lastmins = 0 ;
int mins = 0;
//-------------Variables-----------------
int alarmRing;
char asymbol;
double humidity=0;
double temperature=0;
double light=0;
double voltageOUT=0;
int stopped=0;
int systemTime[3]={0,0,0};
int lastMins=0;
int thresholdTime=20;
//------FUNCTION DECLARATION--------------
void setup(void);
void initGPIO(void);
int main(void);
void getTime(void);
int hexCompensation(int);
int decCompensation(int);
void stop(void);
void reset(void);
void stopAlarm(void);
void clearScreen(void);
double adc(int channel);
void threadADC(void);
void changeFrequency(void);
double vOUT(double adcValue);
void transformADC(void);
void dacOUT(void);
void incrementTime(void);
void setAlarm(void);
void print(void);
//------------MAIN------------------------
int main(){
        alarmRing=0;
        initGPIO();
        setup();
        digitalWrite(buzzer, LOW);
while(1){

        usleep(frequency*1000000);
        getTime();
        transformADC();
        dacOUT();
        incrementTime();
        setAlarm();
        print();
//      std::cout<<voltageOUT<<std::endl;
}
}
//--------FUNCTIONS -----------------------
void getTime(void){
        SS = wiringPiI2CReadReg8(RTC,SEC)-0x80;
        MM = wiringPiI2CReadReg8(RTC,MIN);
        HH = wiringPiI2CReadReg8(RTC,HOUR);
        //HH = hexCompensation(HH);
        //printf("Time: %x:%x:%x\n",HH,MM,SS)
        //std::cout<<HH<<" "<<MM<<" "<<SS<<std::endl;
}

void print(void){
        if(stopped==0){
                 printf("%d:%d:%d\t\t%d:%d:%d\t\t%.1f\t\t%.1f\t%.0f\t%.1f\t%c\n",HH,MM,SS,systemTime[0],systemTime[1],systemTime[2],humidity,temperature,light,voltageOUT,asymbol);
        }
}

void setAlarm(void){
        mins = systemTime[1]*60+systemTime[2];

        if(mins<thresholdTime && (voltageOUT>2.65||voltageOUT<0.65)){
                        alarmRing=1;
                        asymbol='*';
                        digitalWrite(buzzer,HIGH);
        }
        else if ((mins-lastMins)>=thresholdTime&&(voltageOUT>2.65||voltageOUT<0.65)){
                        alarmRing=1;
                        asymbol='*';
                        digitalWrite(buzzer,HIGH);
        }

}

void dacOUT(void){
   voltageOUT=(light/1023)*humidity;
}

void incrementTime(void){
        systemTime[2]+=frequency;
        if(systemTime[2]>=60){
                systemTime[2]=systemTime[2]-60;
                systemTime[1]+=1;
        }
        if(systemTime[1]==60){
                systemTime[1]=0;
                systemTime[0]+=1;
        }

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
void stopAlarm(){
lastMins =systemTime[1]*60+systemTime[2];
long interruptAlarmTime = millis();
        if (interruptAlarmTime - lastInterruptTime>200){
                alarmRing = 0;
                asymbol=' ';
                digitalWrite(buzzer,LOW);
        }
lastInterruptTime=interruptAlarmTime;


}

double adc(int channel){
        return analogRead(BASE+channel);
}

void threadADC(void){
        std::thread t0(adc, 0);
        std::thread t1(adc, 1);
        std::thread t2(adc, 2);
}
double vOUT(double adc){
        return(3.3*(adc/1023));
}
void transformADC(void){
        humidity=vOUT(adc(0));
        //cout<<vOUT(800)<<endl;
        double adcTemp=adc(2);
        double temp=vOUT(adcTemp);
        temperature=(temp-0.5)*100;
        light=adc(1);
        dacOUT();
}

void initGPIO(void){
    wiringPiSetup(); //This is the default mode
    //set all buttons to INPUT mode
    pinMode(stopAlarmSwitch,INPUT);
    pinMode(resetSwitch,INPUT);
    pinMode(freqSwitch,INPUT);
    pinMode(stopSwitch,INPUT);
    pinMode(buzzer,OUTPUT);
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

void reset(void){
        long interruptTime = millis();
                if (interruptTime - lastInterruptTime>200){
                        systemTime[0]=0;
                        systemTime[1]=0;
                        systemTime[2]=0;
                        clearScreen();
                }

        lastInterruptTime=interruptTime;

}


void clearScreen(void)
{
        printf("%d:%d:%d\t\t%d:%d:%d\t\t%.1f\t\t%.1f\t%.0f\t%.1f\t%c\n",HH,MM,SS,systemTime[0],systemTime[1],systemTime[2],humidity,temperature,light,voltageOUT,asymbol);
        if(system("CLS"))
                system("clear");
}

void stop(void){
        long interruptTime = millis();
                if (interruptTime - lastInterruptTime>200){
                        if(stopped==0){
                                stopped=-1;
                        }
                        else{
                                stopped=0;
                        }
        }
        lastInterruptTime=interruptTime;
}

void setup(void){
    wiringPiSetup();
        RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

        mcp3004Setup (BASE, SPI_CHAN);//sets up ADC
printf("RTC Time\tSysTimer\tHumidity\tTemp\tLight\tDAC Out\tAlarm Out\n");
    // set up ADC, DAC, RTC here
}

int hexCompensation(int units){
        /*Convert HEX or BCD value to DEC where 0x45 == 0d45
          This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
          perform operations which work in base10 and not base16 (incorrect logic)
        */
        int unitsU = units%0x10;

        if (units >= 0x50){
                units = 50 + unitsU;
        }
        else if (units >= 0x40){
                units = 40 + unitsU;
        }
        else if (units >= 0x30){
                units = 30 + unitsU;
        }
        else if (units >= 0x20){
                units = 20 + unitsU;
        }
        else if (units >= 0x10){
                units = 10 + unitsU;
        }
        units = units + 2;
        return decCompensation(units);
}

int decCompensation(int units){
        int unitsU = units%10;

        if (units >= 50){
                units = 0x50 + unitsU;
        }
        else if (units >= 40){
                units = 0x40 + unitsU;
        }
        else if (units >= 30){
                units = 0x30 + unitsU;
        }
        else if (units >= 20){
                units = 0x20 + unitsU;
        }
        else if (units >= 10){
                units = 0x10 + unitsU;
        }
        return units;
}


