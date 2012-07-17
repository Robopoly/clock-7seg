#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "hardware.h"

void resetAll(void);
void setTime(void);

char serialData[7];
unsigned char time_seconds = 0, time_minutes = 0, time_hours = 0;

void setup()
{
  // serial connection for time setting
  Serial.begin(9600);

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
    Serial.readBytesUntil(10, serialData, 7);

    time_hours = (serialData[0]-48)*10+serialData[1]-48;
    time_minutes = (serialData[2]-48)*10+serialData[3]-48;
    time_seconds = (serialData[4]-48)*10+serialData[5]-48;

    setTime();
    
    Serial.write("You are awesome!\n");
  }
}

void setTime()
{
  // reset all digits to 0
  resetAll();

  // increment seconds
  unsigned char i;
  for(i=0; i<time_seconds; i++)
  {
    CLK_SEC_PORT |= (1 << CLK_SEC_PIN);
    _delay_us(1);
    CLK_SEC_PORT &= ~(1 << CLK_SEC_PIN);
  }
  for(i=0; i<time_minutes; i++)
  {
    CLK_MIN_PORT |= (1 << CLK_MIN_PIN);
    _delay_us(1);
    CLK_MIN_PORT &= ~(1 << CLK_MIN_PIN);
  }
  for(i=0; i<time_hours; i++)
  {
    CLK_HOUR_PORT |= (1 << CLK_HOUR_PIN);
    _delay_us(1);
    CLK_HOUR_PORT &= ~(1 << CLK_HOUR_PIN);
  }
}

//overflow interrupt vector 
ISR(TIMER2_OVF_vect)
{
  if(time_seconds < 59)
  {
    time_seconds++;
    CLK_SEC_PORT |= (1 << CLK_SEC_PIN);
    _delay_us(100);
    CLK_SEC_PORT &= ~(1 << CLK_SEC_PIN);
  }
  else if(time_minutes < 59)
  {
    time_minutes++;
    time_seconds = 0;
    RST_SEC_PORT |= (1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT &= ~(1 << RST_SEC_PIN);
    _delay_us(100);
    RST_SEC_PORT |= (1 << RST_SEC_PIN);

    CLK_MIN_PORT |= (1 << CLK_MIN_PIN);
    _delay_us(100);
    CLK_MIN_PORT &= ~(1 << CLK_MIN_PIN);
  }
  else if(time_hours < 23)
  {
    time_hours++;
    time_minutes = 0;
    time_seconds = 0;
    
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
    time_seconds = 0;
    time_minutes = 0;
    time_hours = 0;
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

