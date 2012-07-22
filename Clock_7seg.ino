#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "hardware.h"

void resetAll(void);
void setTime(void);

char inputStream[7];
// store hours, minutes and seconds
struct time
{
  unsigned char hours, minutes, seconds;
};

// time instance
time myTime;

void setup()
{
  // serial connection for time setting
  Serial.begin(9600);
  
  // initialize time to 00h00m00s
  myTime.hours = 0;
  myTime.minutes = 0;
  myTime.seconds = 0;

  // init i/o pins
  DDRB = 0b11111000;
  DDRC = 0b00011100;
  DDRD = 0b11110000;

  // enable line pull up
  PORTD |= 1;
  PORTA |= 0b11;

  // disable lamp test (?)
  LAMP_TEST_PORT |= (1 << LAMP_TEST_PIN);

  // reset all digits to 0
  resetAll();

  // set up asyncronous timer
  TIMSK &= ~((1<<TOIE2)|(1<<OCIE2));
  ASSR |= (1<<AS2); 
  TCNT2 = 0;
  // prescaler
  TCCR2 = 0x05;
  //TCCR2 = 0x01;
  while(ASSR & 0x07);
  TIMSK |= (1<<TOIE2);
  asm("SEI");
}

void loop()
{
  // reprogram the clock: enter HHMMSS format
  // ex: for 23h44m12s enter 234412
  if(Serial.available() > 0)
  {
    Serial.readBytesUntil(10, inputStream, 7);
    unsigned char hours, minutes, seconds;
    
    hours = 10*(inputStream[0]-48)+inputStream[1]-48;
    minutes = 10*(inputStream[2]-48)+inputStream[3]-48;
    seconds = 10*(inputStream[4]-48)+inputStream[5]-48;
    
    // test for valid input, needed in case connecting UART programmer sends weird data
    if(hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 && seconds >= 0 && seconds <= 59)
    {
      myTime.hours = hours;
      myTime.minutes = minutes;
      myTime.seconds = seconds;
      
      setTime();
      Serial.write("You are awesome!\n");
    }
    else
    {
      // report error
      Serial.write("Error setting time!\n");
    }
  }
}

void setTime()
{
  // reset all digits to 0
  resetAll();

  // increment seconds
  unsigned char i;
  for(i = 0; i < myTime.seconds; i++)
  {
    CLK_SEC_PORT |= (1 << CLK_SEC_PIN);
    _delay_us(1);
    CLK_SEC_PORT &= ~(1 << CLK_SEC_PIN);
  }
  for(i = 0; i < myTime.minutes; i++)
  {
    CLK_MIN_PORT |= (1 << CLK_MIN_PIN);
    _delay_us(1);
    CLK_MIN_PORT &= ~(1 << CLK_MIN_PIN);
  }
  for(i = 0; i < myTime.hours; i++)
  {
    CLK_HOUR_PORT |= (1 << CLK_HOUR_PIN);
    _delay_us(1);
    CLK_HOUR_PORT &= ~(1 << CLK_HOUR_PIN);
  }
}

//overflow interrupt vector 
ISR(TIMER2_OVF_vect)
{
  if(myTime.seconds < 59)
  {
    myTime.seconds++;
    CLK_SEC_PORT |= (1 << CLK_SEC_PIN);
    _delay_us(100);
    CLK_SEC_PORT &= ~(1 << CLK_SEC_PIN);
  }
  else if(myTime.minutes < 59)
  {
    myTime.minutes++;
    myTime.seconds = 0;
    RST_SEC_PORT |= (1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT &= ~(1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT |= (1 << RST_SEC_PIN);

    CLK_MIN_PORT |= (1 << CLK_MIN_PIN);
    _delay_us(100);
    CLK_MIN_PORT &= ~(1 << CLK_MIN_PIN);
  }
  else if(myTime.hours < 23)
  {
    myTime.hours++;
    myTime.minutes = 0;
    myTime.seconds = 0;
    
    RST_SEC_PORT |= (1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT &= ~(1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT |= (1 << RST_SEC_PIN);
    
    RST_MIN_PORT |= (1 << RST_MIN_PIN);
    _delay_us(100);
    RST_MIN_PORT &= ~(1 << RST_MIN_PIN);
    _delay_us(100);
    RST_MIN_PORT |= (1 << RST_MIN_PIN);

    CLK_HOUR_PORT |= (1 << CLK_HOUR_PIN);
    _delay_us(100);
    CLK_HOUR_PORT &= ~(1 << CLK_HOUR_PIN);
  }
  else
  {
    myTime.seconds = 0;
    myTime.minutes = 0;
    myTime.hours = 0;
    resetAll();
  }
}

void resetAll(void)
{
  //Reset all digits
  RST_MIN_PORT |=(1<<RST_MIN_PIN);
  RST_SEC_PORT |=(1<<RST_SEC_PIN);
  RST_HOUR_PORT|=(1<<RST_HOUR_PIN);
  _delay_us(100);
  RST_MIN_PORT &=~(1<<RST_MIN_PIN);
  RST_SEC_PORT &=~(1<<RST_SEC_PIN);
  RST_HOUR_PORT&=~(1<<RST_HOUR_PIN);
  _delay_us(100);
  RST_MIN_PORT |=(1<<RST_MIN_PIN);
  RST_SEC_PORT |=(1<<RST_SEC_PIN);
  RST_HOUR_PORT|=(1<<RST_HOUR_PIN);
}

