#include "ath.h"

#include <time.h>

#define     EPOCH2000   946684800


#define SCL_FREQ 100000L

//SQW Output Frequency
#define SQW_1 0x10
#define SQW_4k 0x11
#define SQW_8k 0x12
#define SQW_32k 0x13

//Status Register TWSR (Status Register)
#define START_STATUS 0x08
#define STATUS (TWSR & 0xf8)
#define MT_SLA_ACK 0x18
#define MR_SLA_ACK 0x40
#define MT_DATA_ACK 0x28
#define MR_DATA_ACK 0x50

//Control Register TWCR (Control Register)
#define TWI_START 0xa4
#define TWI_STOP 0x94
#define FLAG_SET (TWCR & 0x80)
#define TWI_READY 0x84

//RTC Register Addresses
#define _1307_addr 0xd0 /*DS1307 standard address*/
#define SEC_REG 0x00
#define MIN_REG 0x01
#define HOUR_REG 0x02
#define WDAY_REG 0x03
#define DAY_REG 0x04
#define MON_REG 0x05
#define YEAR_REG 0x06
#define SQW_REG 0x07

char WeekDays [][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint8_t START() {
  TWCR = TWI_START;
  while (!FLAG_SET); // waiting for the interrupt flag set
  if (STATUS != TWI_START)
    return 0;
  return 1;
}

void STOP() {
  TWCR = TWI_STOP;
}

uint8_t  SLA_R(uint8_t addr) {
  TWDR = addr + 1;  //To read from the slave device, LSB = 1
  TWCR = TWI_READY;
  while (!FLAG_SET);

  if (STATUS != MR_SLA_ACK)
    return 0;
  return 1;
}

uint8_t SLA_W(uint8_t addr) {
  TWDR = addr;  //To write on slave device, LSB = 0
  TWCR = TWI_READY;
  while (!FLAG_SET);

  if (STATUS != MT_SLA_ACK)
    return 0;
  return 1;
}

uint8_t  DATA_R_BYTE() {      // for receiving just one byte;
  TWCR = TWI_READY;
  while (!FLAG_SET);
  return TWDR;
}

uint8_t  DATA_W(uint8_t DATA) {
  TWDR = DATA;
  TWCR = TWI_READY;
  while (!FLAG_SET);

  if (STATUS != MT_DATA_ACK)
    return 0;
  return 1;
}

void SET_RTC(uint8_t Register, uint8_t Data) {
  START();
  SLA_W(_1307_addr);
  DATA_W(Register);
  DATA_W(Data);
  STOP();
}


uint8_t READ_RTC(uint8_t Register) {
  uint8_t aux_Data = 0;
  START();
  SLA_W(_1307_addr);
  DATA_W(Register);
  START();
  SLA_R(_1307_addr);
  aux_Data = DATA_R_BYTE();
  STOP();
  return aux_Data;
}

uint8_t Two_Digits(uint8_t value) {   //Separe split the decimal value 
  return ((value >> 4) * 10 + (value & 0x0F));
}

uint8_t DEC2HEX(uint8_t value){   //convert into a hexadecimal number for RTC TIME
return(((value / 10) << 4) | (value % 10));
}

void SET_SEC(uint8_t _sec) {
  uint8_t _sec_conv = 0;
  if (_sec < 60) {
    _sec_conv = DEC2HEX(_sec);//(((_sec / 10) << 4) | (_sec % 10));
    SET_RTC(SEC_REG, _sec_conv);
  }
}
/*________________________________________________________________________________*/

void SET_MIN(uint8_t _min) {
  uint8_t _min_conv = 0;
  if (_min < 60) {
    _min_conv = DEC2HEX(_min);//(((_min / 10) << 4) | (_min % 10));
    SET_RTC(MIN_REG, _min_conv);
  }
}

void SET_HOUR(uint8_t _hour) {
  uint8_t _hour_conv = 0;
  if (_hour <= 24) {
    _hour_conv = DEC2HEX(_hour);//(((_hour / 10) << 4) | (_hour % 10));
    SET_RTC(HOUR_REG, _hour_conv);
  }
}

void SET_DAY(uint8_t _day) {
  uint8_t _day_conv = 0;
  uint8_t aux_month = 0;
  uint8_t aux_year = 0;
  _day_conv = DEC2HEX(_day);//(((_day / 10) << 4) | (_day % 10));

  aux_month = READ_RTC(MON_REG);
  aux_month = (Two_Digits(aux_month));

  if (aux_month == 2) {
    aux_year = READ_RTC(YEAR_REG);
    aux_year = (Two_Digits(aux_year));
    if (aux_year % 4 == 0 && _day <= 29) {
      SET_RTC(DAY_REG, _day_conv);
    }
    else if (_day <= 28) {
      SET_RTC(DAY_REG, _day_conv);
    }
    else;
  }

  else if (aux_month < 8) {
    if (_day <= (30 + (aux_month % 2))) {
      SET_RTC(DAY_REG, _day_conv);
    }
  }

  else if (aux_month <= 12) {
    if (_day <= (30 + ((aux_month + 1) % 2))) {
      SET_RTC(DAY_REG, _day_conv);
    }
  }
}

void SET_MON(uint8_t _month) {
  uint8_t _month_conv = 0;
  if ((_month <= 12) && (_month > 0) ) {
    _month_conv = DEC2HEX(_month);//(((_month / 10) << 4) | (_month % 10));
    SET_RTC(MON_REG, _month_conv);
  }
}

void SET_YEAR(uint8_t _year) {
  uint8_t _year_conv = 0;
  if (_year <= 99) {
    _year_conv = DEC2HEX(_year);//(((_year / 10) << 4) | (_year % 10));
    SET_RTC(YEAR_REG, _year_conv);
  }
}


typedef struct systime {

} systime;
systime stime;




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athrtc_init() {

    DDRD = 0;
    PORTD |= 0x03;   //pull up no 2560
    TWSR = 0;       //
    TWBR = ((F_CPU / SCL_FREQ) - 16) / 2;

    SET_RTC(SQW_REG, SQW_1);

      SET_YEAR(16);
      SET_MON(12);
      SET_DAY(31);
      SET_HOUR(23);
      SET_MIN(59);
      SET_SEC(50);

}

void athrtc_update(double dt) {
    #define HC(hex) hex = ((hex >> 4) * 10 + (hex & 0x0F))
    #define HTC(hex) ((hex >> 4) * 10 + (hex & 0x0F))

    uint8_t hours = READ_RTC(HOUR_REG);
    uint8_t minutes = READ_RTC(MIN_REG);
    uint8_t seconds = READ_RTC(SEC_REG);

    //if (hours & 0x40)
    //  hours &= 0x1f; //12H
    //else
    //  hours &= 0x3f; //24H

    uint8_t days = READ_RTC(DAY_REG);
    uint8_t months = READ_RTC(MON_REG);
    uint8_t years = READ_RTC(YEAR_REG);

    //HC(hours);
    //HC(minutes);
    //HC(seconds);
    //HC(days);
    //HC(months);
    //HC(years);

    /*
    if (year is not divisible by 4) then (it is a common year)
    else if (year is not divisible by 100) then (it is a leap year)
    else if (year is not divisible by 400) then (it is a common year)
    else (it is a leap year)
    */



    struct tm t;
    time_t now;
    t.tm_year = HTC(years) +2000 - 1900 + 30;
    t.tm_mon = HTC(months) - 1;           // Month, 0 - jan
    t.tm_mday = HTC(days);          // Day of the month
    t.tm_hour = HTC(hours);
    t.tm_min = HTC(minutes);
    t.tm_sec = HTC(seconds);
    t.tm_isdst = 1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    now = mktime(&t);

    //1483228800
    // 536544201



    //athlcd_printf(0, "%d:%d:%d", hours, minutes, seconds);
    //athlcd_printf(1, "%d/%d/%d %.6f", days, months, years + 2000, dt);

    //athlcd_printf(0, "%02x%02x%02x %02x%02x%4x", hours, minutes, seconds, days, months, years+0x2000);
    //athlcd_printf(1, "%ld; %d", now, HTC(years));

    //struct tm t;
    //time_t t_of_day;
    //t.tm_year = 2011-1900;
    //t.tm_mon = 7;           // Month, 0 - jan
    //t.tm_mday = 8;          // Day of the month
    //t.tm_hour = 16;
    //t.tm_min = 11;
    //t.tm_sec = 42;
    //t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    //t_of_day = mktime(&t);


}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
