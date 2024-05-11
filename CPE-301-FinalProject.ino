//Authors: Michael Pedote, Thompson Thai, and Z He
//CPE 301 Final Project

#include <LiquidCrystal.h>

volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107;  
volatile unsigned char* pin_k  = (unsigned char*) 0x106;

volatile unsigned char* port_d = (unsigned char*) 0x2B; 
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A;  
volatile unsigned char* pin_d  = (unsigned char*) 0x29;

volatile unsigned char* port_e = (unsigned char*) 0x2E; 
volatile unsigned char* ddr_e  = (unsigned char*) 0x2D;  
volatile unsigned char* pin_e  = (unsigned char*) 0x2C;

#define GREEN_OFF *port_d &= 0b11111110
#define GREEN_ON *port_d |= 0b00000001
#define BLUE_OFF *port_d &= 0b11111101
#define BLUE_ON *port_d |= 0b00000010
#define RED_OFF *port_d &= 0b11111110
#define RED_ON *port_d |= 0b00000100
#define YELLOW_OFF *port_d &= 0b11110111
#define YELLOW_ON *port_d |= 0b00001000
#define FAN_OFF *port_e &= 0b11111110
#define FAN_ON *port_e |= 0b00000001

const int RS = 12, EN = 11, D4 = 5, D5 = 4, D6 = 3, D7 = 2;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    lcd.begin(16,2);
    *ddr_k &= 0xFB;
    *ddr_d |= 0x0F;
    *ddr_e |= 0x03;

    *port_k |= 0x04;
}




void loop() {
  // put your main code here, to run repeatedly:
  if(*pin_k & 0x04){
    //pressed
    FAN_ON;
  }else{
    //not pressed
    FAN_OFF;
  }
}
