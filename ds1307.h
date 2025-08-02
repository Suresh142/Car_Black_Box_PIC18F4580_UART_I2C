#ifndef Ds1307_H
#define Ds1307_H

/* RTC */
#define SLAVE_READ_DS		0xD1
#define SLAVE_WRITE_DS		0xD0

/*EXTERNAL EEPROM */
#define SLAVE_READ_EE       0xA1
#define SLAVE_WRITE_EE      0xA0

#define SEC_ADDR		0x00
#define MIN_ADDR		0x01
#define HOUR_ADDR		0x02
#define DAY_ADDR		0x03
#define DATE_ADDR		0x04
#define MONTH_ADDR		0x05
#define YEAR_ADDR		0x06
#define CNTL_ADDR		0x07

/* RTC */
void init_ds1307();
 void get_time();
void write_ds1307(unsigned char address1,  unsigned char data);
unsigned char read_ds1307(unsigned char address1);

/* EXTERNAL EEPROM */
void write_external_eeprom(unsigned char address1,  unsigned char data);
unsigned char read_external_eeprom(unsigned char address1);

#endif