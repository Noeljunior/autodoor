/* AVR CORE LIBS */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>

/* C LIBS */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                  HW MACRO HELPERS
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "AVRAugment_io.h"
#define HIGH                        0xFF
#define LOW                         0x00

#define PIN_MODE(PIN, MODE)         SET_REG(DDR, PIN, MODE)
#define PIN_DOUT(PIN)               PIN_MODE(PIN, HIGH)
#define PIN_DIN(PIN)                PIN_MODE(PIN, LOW)

#define PIN_SET(PIN, VAL)           SET_REG(PORT, PIN, VAL)
#define PIN_GET(PIN)                (GPIN(PIN) & _BV(P##PIN))
#define PIN_HIGH(PIN)               PIN_SET(PIN, HIGH)
#define PIN_LOW(PIN)                PIN_SET(PIN, LOW)

/* Get's the register of type RG of the pin PIN */
#define GETREG(RG, PIN)             R1_GETREG(RG, PIN)
#define R1_GETREG(RG, PIN)          R2_GETREG(RG, PIN, PIN##_LETTER)
#define R2_GETREG(RG, PIN, LETTER)  R3_GETREG(RG, PIN, LETTER)
#define R3_GETREG(RG, PIN, LETTER)  RG##LETTER

/* Get's the bit of the pin PIN */
#define BIT(PIN)                    R1_BIT(PIN)
#define R1_BIT(PIN)                 PIN##_NUMBER

/* Get's the DDRn/PORTn/PINn of the pin PIN */
#define GDDR(PIN)                   GETREG(DDR, PIN)
#define GPORT(PIN)                  GETREG(PORT, PIN)
#define GPIN(P)                     GETREG(PIN, P)
#define GBIT(P)                     R1_BIT(P)

#define ATHPIN(DEF)                 DEF##_PIN

/* Update Limiter */
#define ATH_UPL_DECLARE(var)        double var
#define ATH_UPL_CHECK(var, hz)      var += dt; if (var < (1.0 / (double) (hz)))\
                                        return
#define ATH_UPL_RESET(var)          var = 0.0

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                      MODULES
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "athall.h"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                        ATH
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef TD_ATHP_MODE_H_
#define TD_ATHP_MODE_H_
typedef enum    ATHP_M {
                    /* three LSBs */
                    ATHP_INPUT          = 1,
                    ATHP_OUTPUT         = 2,
                    ATHP_ANALOG         = 3,
                    ATHP_PWM            = 4,
                    /* 4th bit */
                    ATHP_ACTIVEHIGHT    = (1 << 3),
                    /* 5th bit */
                    ATHP_ACTIVELOW      = (1 << 4),
                    /* 6th bit */
                    ATHP_SETHIGH        = (1 << 5),
                    /* 7th bit */
                    ATHP_SETLOW         = (1 << 6),
                    /* 8th bit */
                    ATHP_INVERT         = (1 << 7),
                } ATHP_M;
#endif
#ifndef TD_ATHP_COUNTER_H_
#define TD_ATHP_COUNTER_H_
typedef enum    ATHP_C { //ATHP_PWM_NONE = 0,
                    /* 8 bit Timer/Counter[02] */
                    ATHP_PWM_0A, ATHP_PWM_0B,
                    ATHP_PWM_2A, ATHP_PWM_2B,

                    /* 16 bit Timer/Counter[1345] */
                    ATHP_PWM_1A, ATHP_PWM_1B, ATHP_PWM_1C,
                    ATHP_PWM_3A, ATHP_PWM_3B, ATHP_PWM_3C,
                    ATHP_PWM_4A, ATHP_PWM_4B, ATHP_PWM_4C,
                    ATHP_PWM_5A, ATHP_PWM_5B, ATHP_PWM_5C,
                ATHP_PWM_MAX } ATHP_C;
#endif

struct pin {
    uint8_t             mode;
    volatile uint8_t    *ddr;
    volatile uint8_t    *port;
    volatile uint8_t    *pin;
    uint8_t             bit;
    uint8_t             mask;
    uint8_t             inv;

    /* pwm specific */
    union pwm {
        volatile uint16_t   *pwm16;
        volatile uint8_t    *pwm8;
        } pwm;
    ATHP_C              tcounter;
    uint16_t            top;
};

#ifndef TD_ATHP_H_
#define TD_ATHP_H_
typedef struct pin pin;
#endif

void            ath_pin_init(pin * p, volatile uint8_t *ddr,
                    volatile uint8_t *port, volatile uint8_t *pin, uint8_t bit);
void            ath_pin_setmode(pin * p, uint8_t mode);
void            ath_pin_activelow(pin * p);
void            ath_pin_activehight(pin * p);
void            ath_init_setmode(pin * p, volatile uint8_t *ddr,
                    volatile uint8_t *port, volatile uint8_t *pin, uint8_t bit,
                    uint8_t mode);
void            ath_pin_high(pin * p);
void            ath_pin_low(pin * p);
uint8_t         ath_pin_read(pin * p);
        /* PWMs */
#define         TOP_F_PS(f, ps)     ((uint16_t) (F_CPU / (f) / ps - 1))
void            ath_init_pwm(pin * p, ATHP_C tcounter, uint16_t top,
                    uint16_t prescaler);
void            ath_pin_pwm16(pin * p, uint16_t dc);
void            ath_pin_pwm8(pin * p, uint8_t dc);
void            ath_pin_pwm(pin * p, double dc);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                     ATH:TIMING
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* DECLARATIONS */
void            athtiming_init();
void            athtiming_update(double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:LCD
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CONFIG */
#define ATHLCD_PIN_RS           A0
#define ATHLCD_PIN_ENABLE       A1
#define ATHLCD_PIN_DATA         A2  /* first bit of the consecutive 4 bits */

/* PARAMETERS */
#define ATHLCD_LINEBUFFER       64

/* DECLARATIONS */
void            athlcd_init();
void            athlcd_update(double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:IN
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CONFIG */
#define ATHIN_OK_PIN            K0
#define ATHIN_CANCEL_PIN        K5
#define ATHIN_UP_PIN            K1
#define ATHIN_DOWN_PIN          K2
#define ATHIN_LEFT_PIN          K3
#define ATHIN_RIGHT_PIN         K4
#define ATHIN_DOORA_PIN         F6
#define ATHIN_PAPERA_PIN        F7

/* PARAMETERS */
#define ATHIN_LPT               0.9        /* long pressed time (s) */
#define ATHIN_LPR               0.1        /* long clicked repeat time (s) */

/* DECLARATIONS */
void            athin_init();
void            athin_update();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:OUT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CONFIG */
#define ATHOUT_LED1_PIN         K6
#define ATHOUT_LED2_PIN         K7
#define ATHOUT_SPEAKER_PIN      F0

/* PARAMETERS */

/* DECLARATIONS */
void            athout_init();
void            athout_update(double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:MOTOR
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CONFIG */
#define ATHMOTOR_AUP_SPEED_PIN      L5  /* DRIVER A */
#define ATHMOTOR_AUP_BRAKE_PIN      L3
#define ATHMOTOR_AUP_DIR_PIN        L7
#define ATHMOTOR_AUP_FAULT_PIN      L1
#define ATHMOTOR_AUP_PWM_PIN        5C
#define ATHMOTOR_ADOWN_SPEED_PIN    L4
#define ATHMOTOR_ADOWN_BRAKE_PIN    L2
#define ATHMOTOR_ADOWN_DIR_PIN      L6
#define ATHMOTOR_ADOWN_FAULT_PIN    L0
#define ATHMOTOR_ADOWN_PWM_PIN      5B

#define ATHMOTOR_BUP_SPEED_PIN      L5  /* DRIVER B */
#define ATHMOTOR_BUP_BRAKE_PIN      L3
#define ATHMOTOR_BUP_DIR_PIN        L7
#define ATHMOTOR_BUP_FAULT_PIN      L1
#define ATHMOTOR_BUP_PWM_PIN        5C
#define ATHMOTOR_BDOWN_SPEED_PIN    L4
#define ATHMOTOR_BDOWN_BRAKE_PIN    L2
#define ATHMOTOR_BDOWN_DIR_PIN      L6
#define ATHMOTOR_BDOWN_FAULT_PIN    L0
#define ATHMOTOR_BDOWN_PWM_PIN      5B

/* PARAMETERS */
#define ATHMOTOR_CALIB_UPF          1.15
#define ATHMOTOR_CALIB_DPF          1.0
#define ATHMOTOR_STRENGTH_PERC      0.75
#define ATHMOTOR_SPEED_FACTOR       0.4
#define ATHMOTOR_SPEED_ABSMAX       0.9
#define ATHMOTOR_SPEED_START        0.15
#define ATHMOTOR_SPEED_STOP         0.0
#define ATHMOTOR_SPEED_BRAKE        0.5
#define ATHMOTOR_SPEED_ACCEL        3.5
#define ATHMOTOR_SPEED_DEACEL       3.5
#define ATHMOTOR_LIMIT_DIST         1.5
#define ATHMOTOR_LIMIT_STOP_DIST    0.02
#define ATHMOTOR_LIMIT_SPEED        0.1


/* DECLARATIONS */
void            athmotor_init();
void            athmotor_update(double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:DECODER
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CONFIG */

/* PARAMETERS */
#define ATHDECODER_PPRA         2000
#define ATHDECODER_PPRB         2000

/* DECLARATIONS */
void            athdecoder_init();
void            athdecoder_update(double dt);




