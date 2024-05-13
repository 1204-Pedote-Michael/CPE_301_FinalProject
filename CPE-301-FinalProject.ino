//Authors: Michael Pedote, Thompson Thai, and Z He
//CPE 301 Final Project

#include <LiquidCrystal.h>
#include <DHT11.h>
#include <Stepper.h>
#include <RTClib.h>

volatile unsigned char* port_k = (unsigned char*) 0x108; 
volatile unsigned char* ddr_k  = (unsigned char*) 0x107;  
volatile unsigned char* pin_k  = (unsigned char*) 0x106;

volatile unsigned char* port_c = (unsigned char*) 0x28; 
volatile unsigned char* ddr_c  = (unsigned char*) 0x27;  
volatile unsigned char* pin_c  = (unsigned char*) 0x26;

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

volatile unsigned char *myTCCR1A = (unsigned char *) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char *) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char *) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char *) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned  int *) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char *) 0x36;

#define GREEN_OFF *port_c &= 0b11111110
#define GREEN_ON *port_c |= 0b00000001
#define BLUE_OFF *port_c &= 0b11111101
#define BLUE_ON *port_c |= 0b00000010
#define RED_OFF *port_c &= 0b11111011
#define RED_ON *port_c |= 0b00000100
#define YELLOW_OFF *port_c &= 0b11110111
#define YELLOW_ON *port_c |= 0b00001000
#define ALL_OFF *port_c &= 0b11110000
#define FAN_OFF *port_c &= 0b11101111
#define FAN_ON *port_c |= 0b00010000

#define RDA 0x80
#define TBE 0x20 

unsigned int currentTicks;
bool timer_running;

const int RS = 12, EN = 11, D4 = 6, D5 = 5, D6 = 4, D7 = 3;

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

DHT11 dht11(2);

const int stepsPerRevolution = 2038;

Stepper myStepper = Stepper(stepsPerRevolution, 7, 9, 8, 10);

enum State {DISABLED, IDLE, ERROR, RUNNING};

RTC_DS1307 rtc;

void setup() {

  U0init(9600);
  lcd.begin(16,2);
  adc_init();

  rtc.begin();

  rtc.adjust(DateTime(2024,5,11,19,0,0));

  attachInterrupt(digitalPinToInterrupt(18),onOffButton,FALLING);
  attachInterrupt(digitalPinToInterrupt(19),resetButton,FALLING);

  *ddr_k &= 0xEF;
  *ddr_c |= 0x1F;

  *port_k |= 0x10;
}

enum State curState = DISABLED, prevState = DISABLED;


void loop() {

  potentiometer();
  
  switch(curState){
    case IDLE:
      ALL_OFF;
      GREEN_ON;
      FAN_OFF;
      
      if(checkTemp()){
        curState = RUNNING;
      }
      if(!checkWater()){
        curState = ERROR;
      }
      break;
    case ERROR:
      ALL_OFF;
      FAN_OFF;
      RED_ON;
      
      lcd.setCursor(0,0);
      lcd.println("ERROR Water Low  ");

      break;
    case RUNNING:
      ALL_OFF;
      FAN_ON;
      BLUE_ON;
      if(!checkWater()){
        curState = ERROR;
      }
      if(!checkTemp()){
        curState = IDLE;
      }
      break;
    case DISABLED:
      ALL_OFF;
      YELLOW_ON;
      FAN_OFF;
      break;
  }
  if(prevState != curState){
    switch(curState){
      case IDLE:
      printString("IDLE ");
      break;
    case ERROR:
      lcd.clear();
      printString("ERROR ");
      break;
    case RUNNING:
      printString("RUNNING ");
      break;
    case DISABLED:
      printString("DISABLED ");
      break; 
    }
    printTime();

  }
  prevState = curState;
}

void potentiometer(){
  int pot = adc_read(10);
  if(pot > 200){
    myStepper.setSpeed(5);
    myStepper.step(100);

  }else if(pot < 50){
    myStepper.setSpeed(5);
    myStepper.step(-100);

  }else{
    myStepper.step(0);
  }
}

bool checkTemp(){
    int temperature = 0;
    int humidity = 0;

    int result = dht11.readTemperatureHumidity(temperature, humidity);

    if (result == 0) {
      lcd.setCursor(0,0);
      lcd.print("Temperature: ");
      lcd.print(temperature);
      lcd.print("C");
      lcd.setCursor(0,1);
      lcd.print("Humidity: ");
      lcd.print(humidity);
      lcd.println("%   ");
    } else {
      lcd.println(DHT11::getErrorString(result));
    }
    if(temperature >= 20){
      return 1;
    }else{
      return 0;
    }
    my_delay(1000);
}

void onOffButton(){
  if(curState != DISABLED){
    curState = DISABLED;
  }else{
    curState = RUNNING;
  }
}

void resetButton(){
  if(checkWater() && curState == ERROR){
    curState = IDLE;
  }
}

bool checkWater(){
  int waterLevel = adc_read(0);
  return waterLevel > 100;
}

void printTime(){
  DateTime now = rtc.now();

  unsigned char charArr[2];
  String str = String(now.month());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.month() >= 10);
  U0putchar('/');  
  str = String(now.day());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.day() >= 10);
  U0putchar('/');
  str = String(now.year());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.year() >= 10);
  U0putchar('   ');  
  str = String(now.hour());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.hour() >= 10);
  U0putchar(':');  
  str = String(now.minute());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.minute() >= 10);
  U0putchar(':');
  str = String(now.second());
  str.toCharArray(charArr, 3);
  printTwo(charArr, now.second() >= 10);
  U0putchar('\n');  
}
void printTwo(char* arr, bool gt){
  if(gt){
    U0putchar(arr[0]);
    U0putchar(arr[1]);
  }else{
    U0putchar('0');
    U0putchar(arr[0]);    
  }
}
void printString(String str){
  char arr[str.length()];
  str.toCharArray(arr,str.length());
  for(int i = 0; i < str.length(); i++){
    U0putchar(arr[i]);
  }
}



void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}

void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}


void my_delay(unsigned int freq)
{
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // stop the timer
  *myTCCR1B &= 0xF8;
  // set the counts
  *myTCNT1 = (unsigned int) (65536 - ticks);
  // start the timer
  * myTCCR1A = 0x0;
  * myTCCR1B |= 0b00000001;
  // wait for overflow
  while((*myTIFR1 & 0x01)==0); // 0b 0000 0000
  // stop the timer
  *myTCCR1B &= 0xF8;   // 0b 0000 0000
  // reset TOV           
  *myTIFR1 |= 0x01;
}
