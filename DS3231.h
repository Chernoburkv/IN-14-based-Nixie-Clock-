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

  The license applies to all part of the library including the
  examples and tools supplied with the library.
*/
#ifndef DS3231_h
#define DS3231_h

#include <avr/io.h>
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#ifndef TWI_FREQ
#define TWI_FREQ 400000L
#endif


#define DS3231_ADDR_R 0xD1
#define DS3231_ADDR_W 0xD0
#define DS3231_ADDR  0x68

#define SQW_RATE_1  0
#define SQW_RATE_1K  1
#define SQW_RATE_4K  2
#define SQW_RATE_8K  3

#define OUTPUT_SQW  0
#define OUTPUT_INT  1

struct Time
{
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  date;
    uint8_t  mon;
    uint16_t year;
    uint8_t  dow;
};
struct DS3231
{
    uint8_t _burstArray[7];
};

void Time_Time(struct Time * xhis);
    void DS3231_begin();
    struct Time DS3231_getTime(struct DS3231 * xhis);
    struct Time DS3231_getundecodedTime(struct DS3231 * xhis);
    void DS3231_setTime(struct DS3231 * xhis,uint8_t hour, uint8_t min, uint8_t sec);
    void DS3231_setDate(struct DS3231 * xhis,uint8_t date, uint8_t mon, uint16_t year);
    void DS3231_setDOW(struct DS3231 * xhis);
    void DS3231_setDOW2(struct DS3231 * xhis,uint8_t dow);
    uint32_t DS3231_getUnixTime(struct Time t);
    void DS3231_enable32KHz(uint8_t enable);
    void DS3231_setOutput(uint8_t enable);
    void DS3231_setSQWRate(uint8_t rate);
    float DS3231_getTemp();


#endif
