/*
  DS3231.cpp - library support for the DS3231 I2C Real-Time Clock
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved

  You can find the latest version of the library at
  http://www.RinkyDinkElectronics.com/

  xhis library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of xhis library requires you to buy a license that
  will allow commercial use. xhis includes using the library,
  modified or not, as a tool to sell products.
0
  The license applies to all part of the library including the
  examples and tools supplied with the library.
*/
#include "DS3231.h" // Include hardware-specific functions for the correct MCU

#define REG_SEC  0x00
#define REG_MIN  0x01
#define REG_HOUR 0x02
#define REG_DOW  0x03
#define REG_DATE 0x04
#define REG_MON  0x05
#define REG_YEAR 0x06
#define REG_CON  0x0e
#define REG_STATUS 0x0f
#define REG_AGING 0x10
#define REG_TEMPM 0x11
#define REG_TEMPL 0x12
#define SEC_1970_TO_2000 946684800

static const uint8_t dim[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/* Public */

void Time_Time(struct Time * xhis)
{
    xhis->year = 2020;
    xhis->mon  = 1;
    xhis->date = 1;
    xhis->hour = 12;
    xhis->min  = 0;
    xhis->sec  = 0;
    xhis->dow  = 4;
}
void _burstRead(struct DS3231 * xhis);
    uint8_t DS3231__readRegister(uint8_t reg);
    void  DS3231__writeRegister(uint8_t reg, uint8_t value);
    uint8_t DS3231__decode(uint8_t value);
    uint8_t DS3231__decodeH(uint8_t value);
    uint8_t DS3231__decodeY(uint8_t value);
    uint8_t DS3231__encode(uint8_t vaule);



struct Time DS3231_getTime(struct DS3231 * xhis)
{
    struct Time t;
    Time_Time(&t);
    DS3231__burstRead(xhis);
    t.sec = DS3231__decode(xhis->_burstArray[0]);
    t.min = DS3231__decode(xhis->_burstArray[1]);
    t.hour = DS3231__decodeH(xhis->_burstArray[2]);
    t.dow = xhis->_burstArray[3];
    t.date = DS3231__decode(xhis->_burstArray[4]);
    t.mon = DS3231__decode((xhis->_burstArray[5])& 127);
    t.year = DS3231__decode(xhis->_burstArray[6])+2000;
    return t;
}


struct Time DS3231_getundecodedTime(struct DS3231 * xhis)
{
    struct Time t;
    Time_Time(&t);
    DS3231__burstRead(xhis);
    t.sec = (xhis->_burstArray[0]);
    t.min = (xhis->_burstArray[1]);
    t.hour = DS3231__decodeH(xhis->_burstArray[2]);
    t.dow = xhis->_burstArray[3];
    t.date = (xhis->_burstArray[4]);
    t.mon = (xhis->_burstArray[5])& 127;
    t.year = (xhis->_burstArray[6]);
    return t;
}






void DS3231_setTime(struct DS3231 * xhis,uint8_t hour, uint8_t min, uint8_t sec)
{
    if (((hour>=0) && (hour<24)) && ((min>=0) && (min<60)) && ((sec>=0) && (sec<60)))
    {
        DS3231__writeRegister(REG_HOUR, DS3231__encode(hour));
        DS3231__writeRegister(REG_MIN, DS3231__encode(min));
        DS3231__writeRegister(REG_SEC, DS3231__encode(sec));
    }
}

void DS3231_setDate(struct DS3231 * xhis,uint8_t date, uint8_t mon, uint16_t year)
{
    if (((date>0) && (date<=31)) && ((mon>0) && (mon<=12)) && ((year>=2000) && (year<2100)))
    {
        year -= 2000;
        DS3231__writeRegister(REG_YEAR, DS3231__encode(year));
        DS3231__writeRegister(REG_MON, DS3231__encode(mon));
        DS3231__writeRegister(REG_DATE, DS3231__encode(date));
    }
}

void DS3231_setDOW(struct DS3231 * xhis)
{   struct Time _t = DS3231_getTime(xhis);
    uint8_t a = (14 -  _t.mon) / 12;
    uint16_t y = _t.year - a;
    uint8_t m =  _t.mon + 12 * a - 2;
DS3231__writeRegister(REG_DOW, ((7000 + (_t.date + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12)) % 7)+1);
}

void DS3231_setDOW2(struct DS3231 * xhis,uint8_t dow)
{
    if ((dow>0) && (dow<8))
        DS3231__writeRegister(REG_DOW, dow);
}


uint32_t DS3231_getUnixTime(struct Time t)
{
    uint16_t dc;
    dc = t.date;
    for (uint8_t i = 0; i<(t.mon-1); i++)
        dc += dim[i];
    if ((t.mon > 2) && (((t.year-2000) % 4) == 0))
        ++dc;
    dc = dc + (365 * (t.year-2000)) + (((t.year-2000) + 3) / 4) - 1;
    return ((((((dc * 24L) + t.hour) * 60) + t.min) * 60) + t.sec) + SEC_1970_TO_2000;
}

void DS3231_enable32KHz(uint8_t enable)
{
    uint8_t _reg = DS3231__readRegister(REG_STATUS);
    _reg &= ~(1 << 3);
    _reg |= (enable << 3);
    DS3231__writeRegister(REG_STATUS, _reg);
}

void DS3231_setOutput(uint8_t enable)
{
    uint8_t _reg = DS3231__readRegister(REG_CON);
    _reg &= ~(1 << 2);
    _reg |= (enable << 2);
    DS3231__writeRegister(REG_CON, _reg);
}

void DS3231_setSQWRate(uint8_t rate)
{
    uint8_t _reg = DS3231__readRegister(REG_CON);
    _reg &= ~(3 << 3);
    _reg |= (rate << 3);
    DS3231__writeRegister(REG_CON, _reg);
}

float DS3231_getTemp()
{
    uint8_t _msb = DS3231__readRegister(REG_TEMPM);
    uint8_t _lsb = DS3231__readRegister(REG_TEMPL);
    return (float)_msb + ((_lsb >> 6) * 0.25f);
}



uint8_t DS3231__decodeH(uint8_t value)
{
    if (value & 128)
        value = (value & 15) + (12 * ((value & 32) >> 5));
    else
        value = (value & 15) + (10 * ((value & 48) >> 4));
    return value;
}

uint8_t DS3231__decode(uint8_t value)
{
    uint8_t decoded = (value & 15) + 10 * ((value & (15 << 4)) >> 4);
    return decoded;
}

uint8_t DS3231__encode(uint8_t value)
{
    uint8_t encoded = ((value / 10) << 4) + (value % 10);
    return encoded;
}

void DS3231_begin()
{
  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1);
  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
  // enable twi module, acks, and twi interrupt
  TWCR = _BV(TWEN) | _BV(TWIE)/* | _BV(TWEA)*/;
}

void DS3231__burstRead(struct DS3231 * myxhis)
{
  // Send start address
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);      // Send START
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = DS3231_ADDR_W;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = 0;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready

  // Read data starting from start address
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);      // Send rep. START
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = DS3231_ADDR_R;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  for (int i=0; i<7; i++)
  {
   TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);        // Send ACK and clear TWINT to proceed
   while ((TWCR & _BV(TWINT)) == 0) {};         // Wait for TWI to be ready
   myxhis->_burstArray[i] = TWDR;
  }
  TWCR = _BV(TWEN) | _BV(TWINT);            // Send NACK and clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready

  TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);         // Send STOP
}

uint8_t DS3231__readRegister(uint8_t reg)
{
 uint8_t readValue=0;


  // Send start address
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);      // Send START
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = DS3231_ADDR_W;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = reg;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready

  // Read data starting from start address
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);      // Send rep. START
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = DS3231_ADDR_R;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Send ACK and clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  readValue = TWDR;
  TWCR = _BV(TWEN) | _BV(TWINT);            // Send NACK and clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready

  TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);         // Send STOP


 return readValue;
}

void DS3231__writeRegister(uint8_t reg, uint8_t value)
{

  // Send start address
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);      // Send START
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = DS3231_ADDR_W;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = reg;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready
  TWDR = value;
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);         // Clear TWINT to proceed
  while ((TWCR & _BV(TWINT)) == 0) {};          // Wait for TWI to be ready

  TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);         // Send STOP



}
