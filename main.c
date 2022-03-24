#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "DS3231.h"

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define LATCH (1<< PORTB1)     // LATCH
#define SS (1<< PORTB2)       // SS (NC)
#define DATA (1<< PORTB3)    //MOSI (SI)
#define CLOCK (1<< PORTB5)  //SCK (SCK)

#define BUTTON_REGISTER DDRC
#define BUTTON_PIN PINC
#define B0 (1 << PC0)    //0
#define B1 (1 << PC1)    //1
#define B2 (1 << PC2)    //2
#define B3 (1 << PC3)    //3

uint8_t dispstat=0;
uint8_t dispon=0;
uint8_t setstat=0;
uint8_t screen=0;
uint8_t sec_set=0;
uint8_t min_set=0;
uint8_t hour_set=12;
uint8_t date_set=1;
uint8_t mon_set=1;
uint8_t year_set=20;
uint8_t secdigit=0;

struct DS3231 rtc;
struct Time t;


uint8_t s10[] =
{
    0b00000100, //0
    0b00001000, //1
    0b00010000, //2
    0b00100000, //3
    0b01000000, //4
    0b10000000, //5
    0b00000000, //6--
    0b00000000, //7--
    0b00000001, //8
    0b00000010, //9
    0b00000000
};

uint8_t s11[] =
{
    0b00000000, //0--
    0b00000000, //1--
    0b00000000, //2--
    0b00000000, //3--
    0b00000000, //4--
    0b00000000, //5--
    0b01000000, //6
    0b10000000, //7
    0b00000000, //8--
    0b00000000, //9--
    0b00000000
};

uint8_t s20[] =
{
    0b00000001, //0
    0b00000010, //1
    0b00000100, //2
    0b00001000, //3
    0b00010000, //4
    0b00100000, //5
    0b00000000, //6--
    0b00000000, //7--
    0b00000000, //8--
    0b00000000, //9--
    0b00000000
};

uint8_t s21[] =
{
    0b00000000, //0--
    0b00000000, //1--
    0b00000000, //2--
    0b00000000, //3--
    0b00000000, //4--
    0b00000000, //5--
    0b00010000, //6
    0b00100000, //7
    0b01000000, //8
    0b10000000, //9
    0b00000000
};

uint8_t s30[] =
{
    0b00000000, //0--
    0b00000000, //1--
    0b00000000, //2--
    0b00000000, //3--
    0b00000001, //4
    0b00000010, //5
    0b00000000, //6--
    0b00000000, //7--
    0b00000000, //8--
    0b00000000, //9--
    0b00000000
};

uint8_t s31[] =
{
    0b00010000, //0
    0b00100000, //1
    0b01000000, //2
    0b10000000, //3
    0b00000000, //4--
    0b00000000, //5--
    0b00000001, //6
    0b00000010, //7
    0b00000100, //8
    0b00001000, //9
    0b00000000
};

uint8_t s50[] =
{
    0b00000000, //0--
    0b00000000, //1--
    0b00000001, //2
    0b00000010, //3
    0b00000100, //4
    0b00001000, //5
    0b00000000, //6--
    0b00000000, //7--
    0b00000000, //8--
    0b00000000, //9--
    0b00000000
};

uint8_t s51[] =
{
    0b01000000, //0
    0b10000000, //1
    0b00000000, //2--
    0b00000000, //3--
    0b00000000, //4--
    0b00000000, //5--
    0b00000100, //6
    0b00001000, //7
    0b00010000, //8
    0b00100000, //9
    0b00000000
};

int main(void)
{
  SHIFT_REGISTER|=(DATA | LATCH | CLOCK | SS); //Set control pins as outputs
  SHIFT_PORT &= ~(DATA | LATCH | CLOCK | SS); //Set control pins low
  SPCR=(1<< SPE) | (1<< MSTR) | (1<< SPR0) | (1<< SPR1);
  _delay_us(500);
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//1
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//2
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//3
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//4
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//5
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//6
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//7
  SPDR = 0b00000000;
  while(!(SPSR & (1<<SPIF)));//8
  SHIFT_PORT |= (LATCH); //высокий уровень
  _delay_us(50);
  SHIFT_PORT &= ~(LATCH); //низкий уровень

  DS3231_begin();

  BUTTON_REGISTER &= ~(B0 | B1 | B2 | B3);

  cli();
   TCNT1  = 0;
   TCCR1A = 0;
   TCCR1B = 0;
   OCR1A = 62499;            // compare match register 16MHz/256/4Hz
   TCCR1B = ((1 << WGM12) | (1 << CS12) | (0 << CS11) | (0 << CS10));    // 256
   TIMSK = (1 << OCIE1A);
  sei();

    while(1)
    {

    }
}

void nix3x2numTo8x8Bits(uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4, uint8_t n5, uint8_t n6)
{
  SPDR =(s10[n1]); //1
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s11[n1])|(s20[n2]); //2
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s21[n2])|(s30[n3]); //3
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s31[n3]); //4
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish


  SPDR = (s10[n4]); //5
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s11[n4])|(s50[n5]); //6
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s51[n5])|(s30[n6]); //7
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish

  SPDR = (s31[n6]); //8
  while(!(SPSR & (1<< SPIF))); //Wait for SPI process to finish
}

uint8_t number_days(uint8_t m, uint16_t y)
	{
		y=y+2000;
		uint16_t leap = (1 - (y % 4 + 2) % (y % 4 + 1)) * ((y % 100 + 2) % (y % 100 + 1)) + (1 - (y % 400 + 2) % (y % 400 + 1));
		return 28 + ((m + (m / 8)) % 2) + 2 % m + ((1 + leap) / m) + (1/m) - (leap/m);
	}

ISR(TIMER1_COMPA_vect) //This is the interrupt request, and the link is a good help about it --- https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328
{
      if (!(PINC&B3))
             {
              if (dispstat<1){dispstat++;}
              else {
                     if (setstat==6)
                        {
                         DS3231_setTime(&rtc, hour_set, min_set, sec_set);
                         DS3231_setDate(&rtc, date_set, mon_set, 2000+year_set);
                         DS3231_setDOW(&rtc);
                        }
                        dispstat=0;
                        setstat=0;
                    }
              }

switch (dispstat)
{
     case 0 :
          if (!(PINC&B0))
             {
               if (screen<4){screen++;}
               else {screen=0;}
             }

          if (!(PINC&B1))
             {
               if (screen>0){screen--;}
               else {screen=3;}
             }


           t = DS3231_getTime(&rtc);
           secdigit = t.sec%10;
           //dispon=~((t.hour>=0)&&(t.hour<=4));
           switch (screen)
             {
               case 0:

                     if      ((t.sec<10)&&((t.min & 0b00000001)==1))
                             {
                              nix3x2numTo8x8Bits(secdigit, secdigit, secdigit, secdigit, secdigit, secdigit);
                             }
                     else if (((t.sec>=14)&&(t.sec<=15))||((t.sec>=44)&&(t.sec<=45)))
                             {
                              nix3x2numTo8x8Bits((t.year-2000)/10, (t.year-2000)%10, t.mon/10, t.mon%10, t.date/10, t.date%10);
                             }
                     else
                             {
                              nix3x2numTo8x8Bits(t.hour/10, t.hour%10, t.min/10, t.min%10, t.sec/10, secdigit);
                             }

               break;

               case 1:
                              nix3x2numTo8x8Bits(t.hour/10, t.hour%10, t.min/10, t.min%10, t.sec/10, secdigit);
               break;

               case 2:
                              nix3x2numTo8x8Bits((t.year-2000)/10, (t.year-2000)%10, t.mon/10, t.mon%10, t.date/10, t.date%10);
               break;

               case 3:
                              nix3x2numTo8x8Bits(((t.dow+5)%7)+1,10, 10, 10, 10, 10);
               break;

               case 4:
                              nix3x2numTo8x8Bits(secdigit, secdigit, secdigit, secdigit, secdigit, secdigit);
               break;


             }

     break;

     case 1 :
          if (!(PINC&B2))
             {
              if (setstat<6){setstat++;}
              else {setstat=0;}
             }

           switch (setstat)
           {
           case 0:

               if (!(PINC&B0))
                  {
                   if (hour_set<23){hour_set++;}
                   else {hour_set=0;}
                  }

               if (!(PINC&B1))
                  {
                   if (hour_set>0){hour_set--;}
                   else {hour_set=23;}
                  }
           nix3x2numTo8x8Bits(10, 10, 10, 10, hour_set/10, hour_set%10);
           break;
           case 1:
              if (!(PINC&B0))
                 {
                  if (min_set<59){min_set++;}
                  else {min_set=0;}
                 }

               if (!(PINC&B1))
                  {
                   if (min_set>0){min_set--;}
                   else {min_set=59;}
                  }
           nix3x2numTo8x8Bits(10, 10, hour_set/10, hour_set%10, min_set/10, min_set%10);
           break;
           case 2:
              if (!(PINC&B0))
                 {
                  if (sec_set<59){sec_set++;}
                  else {sec_set=0;}
                 }

               if (!(PINC&B1))
                  {
                   if (sec_set>0){sec_set--;}
                   else {sec_set=59;}
                  }
           nix3x2numTo8x8Bits(hour_set/10, hour_set%10, min_set/10, min_set%10, sec_set/10,sec_set%10);
           break;
           case 3:
              if  (!(PINC&B0))
                  {
                   if (year_set<99){year_set++;}
                   else {year_set=0;}
                   if (date_set>number_days(mon_set,year_set)) {date_set=number_days(mon_set,year_set);}
                  }

               if (!(PINC&B1))
                  {
                   if (year_set>0){year_set--;}
                   else {year_set=99;}
                   if (date_set>number_days(mon_set,year_set)) {date_set=number_days(mon_set,year_set);}
                  }
           nix3x2numTo8x8Bits(10, 10, 10, 10, year_set/10,year_set%10);
           break;
           case 4:
              if (!(PINC&B0))
                 {
                  if (mon_set<12){mon_set++;}
                  else {mon_set=1;}
                  if (date_set>number_days(mon_set,year_set)) {date_set=number_days(mon_set,year_set);}
                 }

              if (!(PINC&B1))
                 {
                  if (mon_set>1){mon_set--;}
                  else {mon_set=12;}
                  if (date_set>number_days(mon_set,year_set)) {date_set=number_days(mon_set,year_set);}
                 }
           nix3x2numTo8x8Bits(10, 10, year_set/10,year_set%10, mon_set/10,mon_set%10);
           break;
           case 5:
              if (!(PINC&B0))
                 {
                  if (date_set<number_days(mon_set,year_set)){date_set++;}
                  else {date_set=1;}
                 }

              if (!(PINC&B1))
                 {
                   if (date_set>1){date_set--;}
                   else {date_set=number_days(mon_set,year_set);}
                 }

            nix3x2numTo8x8Bits(year_set/10,year_set%10, mon_set/10,mon_set%10, date_set/10, date_set%10);
            break;
            case 6:
            nix3x2numTo8x8Bits(8,8,8,8,8,8);
            break;
            }
         break;
        }
      SHIFT_PORT |= LATCH; //Toggle latch to copy new data from the storage register to output register
      _delay_us(50);
      SHIFT_PORT &= ~LATCH;//data transmition from mc to the storage register

}












