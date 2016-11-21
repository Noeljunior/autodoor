/* AVR CORE LIBS */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/eeprom.h>

/* C LIBS */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#define ATHE_EEP(type, name)        const type EEMEM NV_##name
#define ATHE_EEP_LOAD(type, name)   const type NV_##name

#define ATH_RESET_EEPROM


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                  PUBLIC INTERFACE
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef TD_ATH_S_
#define TD_ATH_S_
typedef struct semaphore {
    uint8_t lock;
    uint8_t who;
} semaphore;
#endif

#define         ATH_NOSIDE              -1
#define         ATH_SIDEA               0
#define         ATH_SIDEB               1
#define         ATH_SIDES               2

void            athinit();
void            athupdate();

double          ath_dt();

void            ath_seminit(semaphore * s);
void            ath_sempost(semaphore * s);
uint8_t         ath_semwait(semaphore * s, uint8_t w);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:WARRANTY
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef TD_ATHWAR_H_
#define TD_ATHWAR_H_
typedef enum    ATHWAR {
                    ATHWAR_CLEAR        = 0,
                    ATHWAR_VOIDED       = 1 << 0,
                    ATHWAR_RUNTIME      = 1 << 1,
                    ATHWAR_TIMEOUT      = 1 << 2,
                    ATHWAR_TIMESHIFT    = 1 << 3,
                    ATHWAR_HWERROR      = 1 << 4,
                ATHWAR_MAX } ATHWAR;
#endif

int8_t          athwarranty_init();
int8_t          athwarranty_update(double dt);
int8_t          athwarranty_check();
void            athwarranty_void(ATHWAR reason);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:EEPROM
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void            ath_eeprom_init();
int8_t          ath_eeprom_register(void * obj, uint16_t size);
void            ath_eeprom_reload(uint8_t oid);
void            ath_eeprom_save(uint8_t oid);
void            ath_eeprom_saveall();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:TIMING
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32_t        athtiming_ms();
uint32_t        athtiming_us();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:LCD
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void            athlcd_printf(uint8_t line, const char * format, ...);
void            athlcd_clear();


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:IN
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef TD_ATHIN_H_
#define TD_ATHIN_H_
typedef enum    ATHIN {
                    ATHIN_OK,
                    ATHIN_CANCEL,
                    ATHIN_UP,
                    ATHIN_DOWN,
                    ATHIN_LEFT,
                    ATHIN_RIGHT,
                    ATHIN_DOOR,
                    ATHIN_PAPER,
                    ATHIN_WARRANTY,
                ATHIN_MAX } ATHIN;
#endif

void            athin_reset(uint8_t in);
void            athin_ignrel(uint8_t in);
uint8_t         athin_clicked(uint8_t in);
uint8_t         athin_longclicked(uint8_t in);
uint8_t         athin_exclicked(uint8_t in);
uint8_t         athin_released(uint8_t in);
uint8_t         athin_clicking(uint8_t in);
uint8_t         athin_pressed(uint8_t in);
uint8_t         athin_longpressed(uint8_t in);
uint8_t         athin_switchedon(uint8_t in);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:OUT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHOUT_REPEAT_NO_OFF            0
#define ATHOUT_REPEAT_NO_ON             1
#define ATHOUT_REPEAT_YES               2

#ifndef TD_ATHOUT_H_
#define TD_ATHOUT_H_
typedef enum    ATHOUT {
                    ATHOUT_SPEAKER,
                    ATHOUT_RELAY,
                    ATHOUT_LCDBL,
                    ATHOUT_LCDCONTRAST,
                ATHOUT_MAX } ATHOUT;
#endif

void            athout_on(uint8_t out);
void            athout_off(uint8_t out);
void            athout_blink(uint8_t out, double f, double t, uint8_t r);
void            athout_sequence(uint8_t out, double * v, uint16_t vs, uint8_t r);
void            athout_dc(uint8_t out, double dc);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:RGB
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef ATH_USE_RGB
#define         ATHRGB_RGB      0
#define         ATHRGB_HSV      1
#define         ATHRGB_HSL      2
#ifndef TD_ATHRTGB_H_
#define TD_ATHRTGB_H_
typedef enum    ATHRGB {
                    ATHRGB_P1,
                ATHRGB_MAX } ATHRGB;
#endif

void            athrgb_calib(ATHRGB rgb, double cr, double cg, double cb);
void            athrgb_ramp(ATHRGB rgb, double rr, double rg, double rb);

void            athrgb_rgb(ATHRGB rgb, double r, double g, double b);
void            athrgb_hsv(ATHRGB rgb, double h, double s, double v);
void            athrgb_hsl(ATHRGB rgb, double h, double s, double l);

void            athrgb_fadeto(ATHRGB rgb, double a, double b, double c, double dt, uint8_t t);
void            athrgb_flicker_on(ATHRGB rgb);
void            athrgb_flicker_off(ATHRGB rgb);
uint8_t         athrgb_fading(ATHRGB rgb);

void            HSVtoRGB(double h, double s, double v, double *rgb);
void            HSLtoRGB(double h, double s, double l, double *rgb);
//void            athout_on(uint8_t out);
//void            athout_off(uint8_t out);
//void            athout_blink(uint8_t out, double f, double t, uint8_t r);
//void            athout_sequence(uint8_t out, double * v, uint16_t vs, uint8_t r);
//void            athout_dc(uint8_t out, double dc);

#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:MOTOR
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define         ATHM_NORMAL             1.0
#define         ATHM_FAST               1.1
#define         ATHM_SLOW               0.8

#define         ATHM_ONESHOT            0
#define         ATHM_STICKY             1

#define         ATHM_SIDEA              ATH_SIDEA
#define         ATHM_SIDEB              ATH_SIDEB


#ifndef TD_ATHMOTOR_H_
#define TD_ATHMOTOR_H_
/*       N/A      HOW  WHERE
    |__.__.__.__:__.__:__.__|
*/
typedef enum    ATHMOTOR {
                    /*
                     *      operation modes
                     */
                    /* two LSB */
                    ATHM_UP             = 0,
                    ATHM_DOWN           = 1,
                    ATHM_STOP           = 2,
                    ATHM_BRAKE          = 3,
                    /* 3rd bit */
                    ATHM_SMOOTH         = (0 << 2),
                    ATHM_HARD           = (1 << 2),
                } ATHMOTOR;
#endif
#ifndef TD_ATHMOTORM_H_
#define TD_ATHMOTORM_H_
typedef enum    ATHMOTORO {
                    ATHM_BOTTOM         = -1,
                    ATHM_BOTH           = 0,
                    ATHM_TOP            = 1,
                } ATHMOTORO;
#endif
void            athmotor_gos(uint8_t side, uint8_t wherehow, double speedfactor);
void            athmotor_go(uint8_t side, uint8_t wherehow);
void            athmotor_gotos(uint8_t side, double towhere, uint8_t sticky,
                                double speedfactor);
void            athmotor_goto(uint8_t side, double towhere, uint8_t sticky);
void            athmotor_which(uint8_t side, ATHMOTOR which);
double          athmotor_position(uint8_t side);
double          athmotor_rps(uint8_t side);
uint8_t         athmotor_targeted(uint8_t side);
void            athmotor_set_limits(uint8_t side, double start, double end);
void            athmotor_unset_limits(uint8_t side);
uint8_t         athmotor_islimited(uint8_t side);
void            athmotor_set_dirs(uint8_t a_up, uint8_t a_down, uint8_t b_up,
                                uint8_t b_down);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:DECODER
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define         ATHD_SIDEA              ATH_SIDEA
#define         ATHD_SIDEB              ATH_SIDEB

int32_t         athdecoder_read(uint8_t side);
int32_t         athdecoder_diff(uint8_t side);
double          athdecoder_position(uint8_t side);
double          athdecoder_rps(uint8_t side);
double *        athdecoder_getposition(uint8_t side);
double *        athdecoder_getrps(uint8_t side);
void            athdecoder_reset(uint8_t side);


