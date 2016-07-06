/*----------------------*/
//DRIVER 1
#define DRV_1A_SPEED PL4
#define DRV_1A_BRAKE PL2
#define DRV_1A_DIR PL6
#define DRV_1A_FAULT PL0

#define DRV_1B_SPEED PL5
#define DRV_1B_BRAKE PL3
#define DRV_1B_DIR PL7
#define DRV_1B_FAULT PL1

//DRIVER 2
/*
#define DRV_2A_SPEED PB7
#define DRV_2A_BRAKE PH7
#define DRV_2A_DIR PG3
#define DRV_2A_FAULT PL3

#define DRV_2B_SPEED PE6
#define DRV_2B_BRAKE PG4
#define DRV_2B_DIR PL0
#define DRV_2B_FAULT PL1
*/
/*----------------------*/


//ENCODER HCTL-2032
#define ENC_D0 PC0
#define ENC_D1 PC1
#define ENC_D2 PC2
#define ENC_D3 PC3
#define ENC_D4 PC4
#define ENC_D5 PC5
#define ENC_D6 PC6
#define ENC_D7 PC7

#define ENC_OE PG0
#define ENC_SEL1 PG1
#define ENC_SEL2 PG2
//#define ENC_EN1 PG2
//#define ENC_EN2 PJ6
#define ENC_RST_X PD7
//#define ENC_RST_Y PG1
//#define ENC_XY PA7

/*---------------------*/
//RTC
#define RTC_SCLK PF2
#define RTC_IO PF1
#define RTC_CE PF0

/*---------------------*/
//USER INTERFACE
#define LCD_ENABLE PA1
#define LCD_RS PA0
#define LCD_D4 PA2
#define LCD_D5 PA3
#define LCD_D6 PA4
#define LCD_D7 PA5

#define KB_OK PK4           // |o|
#define KB_CANCEL PK3       // |o|
#define KB_UP PK2           // |o| |D (Yellow)
#define KB_DOWN PK1         // |o| |D (Yellow)
#define KB_MODE PK0           // |o| |D (Red)

#define LED_YELLOW_DOWN PK5       // |D DOWN (Yellow)
#define LED_YELLOW_UP PK6       // |D UP (Yellow) 
#define LED_RED_OK PK7       // |D (Red)

#define SPEAKER PB4
#define SPARE_AN PK1

/*-------------------*/
//SERIAL_0
#define RXD0 PE0
#define TXD0 PE1
#define XCK0 PE2

/*-------------------*/
//SERIAL_1
#define RXD1 PD2
#define TXD1 PD3
#define XCK1 PD5

/*-------------------*/
//SERIAL_2
#define RXD2 PH0
#define TXD2 PH1
#define XCK2 PH2

/*-------------------*/
//SERIAL_3
#define RXD3 PJ0
#define TXD3 PJ1
#define XCK3 PJ2

/*-------------------*/
//ICSP
#define MISO PB3
#define MOSI PB2
#define SCK PB1

/*-------------------*/
//LIGHT
#define LIGHT_EN PK0
#define LIGHT_PWM1 PE3
#define LIGHT_PWM2 PE4
#define LIGHT_LDR PF3

/*------------------*/
//POWER SYSTEM
#define SYS_POWER PF4
