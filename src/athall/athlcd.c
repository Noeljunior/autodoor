#include "ath.h"

#define FPS         30.0

/* TODO
    * implement some kind of vertical autoscroll
    * update one line at a time; do not update both if one didn't change
    * clear lcd more efficient
    * use new pin API
    * implement some kind of output buffer
*/


/* LCD PINOUT */
/*
    1  GND          -
    2  VCC          -
    3  Contrast     -
    4  RS           A0  PC0
    5  RW           -
    6  ENABLE       A1  PC1
    7  DB0          -
    8  DB1          -
    9  DB2          -
    10 DB3          -
    11 DB4          A2 PC2
    12 DB5          A3 PC3
    13 DB6          A4 PC4
    14 DB7          A5 PC5
    15 BLED+        -
    16 BLED-        -
*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void            sendcommand(uint8_t value);
void            writeword(uint8_t value);

/* private globals */
char            lcdbuffer[2][ATHLCD_LINEBUFFER + 1];
char            lcddbuffer[2][16 + 1];
uint8_t         doupdate;

ATH_UPL_DECLARE(wait);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define _SETDATA(P, VAL) GPORT(P) = (GPORT(P) & ~(0x0F << BIT(P))) | ((VAL) << BIT(P));
#define PIN_PULSE(PIN, DELAY)\
                                PIN_HIGH(PIN);\
                                _delay_us(DELAY);\
                                PIN_LOW(PIN)

void athlcd_init() {
    /* PIN MODES: digital out */
    PIN_DOUT(ATHLCD_PIN_RS);
    PIN_DOUT(ATHLCD_PIN_ENABLE);

    GDDR(ATHLCD_PIN_DATA) |= 0x0F << BIT(ATHLCD_PIN_DATA);

    /* set all outputs to low */
    PIN_LOW(ATHLCD_PIN_RS);
    PIN_LOW(ATHLCD_PIN_ENABLE);
    GPORT(ATHLCD_PIN_DATA) &= ~(0x0F << BIT(ATHLCD_PIN_DATA));

    /* init internal data struct */
    doupdate = 1;
    lcdbuffer[0][0] = lcdbuffer[1][0] =
    lcddbuffer[0][0] = lcddbuffer[1][0] = '\0';

    /* * * * * * * * * * * * * * * * * * * * INIT HW */
    /* power on delay */
    _delay_ms(15);

    _SETDATA(ATHLCD_PIN_DATA, 0x03);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);
    _delay_ms(5);

    _SETDATA(ATHLCD_PIN_DATA, 0x03);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);
    _delay_us(100);

    _SETDATA(ATHLCD_PIN_DATA, 0x03);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);
    _delay_ms(5);

    _SETDATA(ATHLCD_PIN_DATA, 0x02);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);
    _delay_us(40);

    /* (LCD_FUNCTIONSET | LCD_5x8DOTS | LCD_2LINE */
    sendcommand(0x20 | 0x00 | 0x08);
    /* LCD_DISPLAYCONTROL | LCD_DISPLAYON */
    sendcommand(0x08 | 0x04);
    /* LCD_ENTRYMODESET | LCD_ENTRYLEFT */
    sendcommand(0x04 | 0x02);

    /* LCD_CLEAR */
    sendcommand(0x01);
    _delay_ms(2);
}

void athlcd_update(double dt) {
    ATH_UPL_CHECK(wait, FPS);

    if (!doupdate) return;
    doupdate = 0;

    sprintf(lcddbuffer[0], "%-16s", lcdbuffer[0]);
    sprintf(lcddbuffer[1], "%-16s", lcdbuffer[1]);

    int i;
    sendcommand(0x80 | (0));
    for (i = 0; i < 16; i++)
        writeword(lcddbuffer[0][i]);
    sendcommand(0x80 | (0x40));
    for (i = 0; i < 16; i++)
        writeword(lcddbuffer[1][i]);

    ATH_UPL_RESET(wait);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void athlcd_printf(uint8_t line, const char * format, ...) {
    va_list args;
    va_start(args, format);

    vsnprintf(lcdbuffer[line], ATHLCD_LINEBUFFER, format, args);
    // TODO if will scroll, set scroller to 0

    /* set to update */
    doupdate = 1;

    va_end(args);
}

void athlcd_clear() {
    // TODO this is not the way
    athlcd_printf(0, "");
    athlcd_printf(1, "");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void sendcommand(uint8_t value) {
    PIN_LOW(ATHLCD_PIN_RS);

    _SETDATA(ATHLCD_PIN_DATA, value >> 4);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);

    _SETDATA(ATHLCD_PIN_DATA, value);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);

    _delay_us(40);
}


void writeword(uint8_t word) {
    PIN_HIGH(ATHLCD_PIN_RS);

    _SETDATA(ATHLCD_PIN_DATA, word >> 4);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);

    _SETDATA(ATHLCD_PIN_DATA, word);
    PIN_PULSE(ATHLCD_PIN_ENABLE, 1);

    _delay_us(40);
}










