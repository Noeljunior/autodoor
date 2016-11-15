#include "ath.h"

/* TODO
    - 
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define FREQUENCYOUT    30
#define TIMEREPEAT      -1.0

/* MODE STATES */
#define SNN             (0)         /* no output */

/* OPERATION MODES */
#define DIGITAL         (0)         /* if it is a digital */
#define PWM             (1 << 0)    /* if it is a pwm */
#define SWITCH          (1 << 1)    /* on/off output */

#define TRANS_SIN(x, f) (sin((x) * (f) * 2 * M_PI))

typedef enum    state {
                    OFF,
                    ON,
                    BLINK,
                    SEQUENCE,
                } state;

typedef struct {
    pin         pin;
    uint8_t     mode;
    state       state;
    state       tstate;
    //uint8_t     state;
    /* effect control */
    double      dt;
    uint16_t    i;

    /* self blinking */
    double      f;
    double      t;

    /* remote blinking */
    double      *vals;
    uint16_t    vsize;
} out;
out             outs[ATHOUT_MAX] = {0};

void            outinit(out * o, uint8_t mode);
void            outupdate(out * o, double dt);
void            outupdate_pwm(out * o, double dt);

double      SEQ_2BIP[] =  {0.1, 0.9, 0.1, 0.9};
double      SEQ_3BIP[] =  {0.1, 0.1, 0.1, 0.1};

ATH_UPL_DECLARE(waitout);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void athout_init() {
    /* ATHOUT_SPEAKER */
    ath_init_setmode(&outs[ATHOUT_SPEAKER].pin, GALL(ATHOUT_SPEAKER_PIN),
        ATHP_OUTPUT);
    ath_init_pwm(&outs[ATHOUT_SPEAKER].pin, ATHOUT_SPEAKER_PWM,
        TOP_F_PS(ATHOUT_SPEAKER_FREQ_HZ, 1), 1);

    /* ATHOUT_RELAY */
    ath_init_setmode(&outs[ATHOUT_RELAY].pin, GALL(ATHOUT_RELAY_PIN),
        ATHP_OUTPUT | ATHP_SETLOW);
    outinit(&outs[ATHOUT_RELAY], DIGITAL);

}


void athout_update(double dt) {
    //ath_pin_high(&outs[ATHOUT_LED2].pin);
    int i;

    //ATH_UPL_CHECK(waitout, FREQUENCY);

    for (i = 0; i < ATHOUT_MAX; i++) {
        if (outs[i].mode == PWM) {
            outupdate_pwm(&outs[i], dt);
        } else {
            outupdate(&outs[i], dt);
        }
    }

    //ATH_UPL_RESET(wait);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athout_on(uint8_t out) {
    outs[out].state  = ON;
    outs[out].tstate = ON;
    //ath_pin_high(&outs[out].pin);
    ath_pin_set(&outs[out].pin, ATH_HIGH);
    outs[out].i = 0;
}

void athout_off(uint8_t out) {
    outs[out].state  = OFF;
    outs[out].tstate = OFF;
    //ath_pin_low(&outs[out].pin);
    ath_pin_set(&outs[out].pin, ATH_LOW);
    outs[out].i = 0;
}

void athout_blink(uint8_t out, double f, double t, uint8_t r) {
    outs[out].state  = BLINK;
    if (r == ATHOUT_REPEAT_NO_OFF) outs[out].tstate = OFF;
    if (r == ATHOUT_REPEAT_NO_ON)  outs[out].tstate = ON;
    if (r == ATHOUT_REPEAT_YES)    outs[out].tstate = BLINK;

    outs[out].f      = 0.5 / f;
    outs[out].t      = t > 0.0 ? t : (((uint16_t) t) * outs[out].f);
    outs[out].i      = 0;
    outs[out].dt     = outs[out].f;
}

void athout_sequence(uint8_t out, double * v, uint16_t vs, uint8_t r) {
    outs[out].state  = SEQUENCE;
    if (r == ATHOUT_REPEAT_NO_OFF) outs[out].tstate = OFF;
    if (r == ATHOUT_REPEAT_NO_ON)  outs[out].tstate = ON;
    if (r == ATHOUT_REPEAT_YES)    outs[out].tstate = SEQUENCE;

    outs[out].dt    = 0.0;
    outs[out].i     = 0;
    outs[out].vals  = v;
    outs[out].vsize = vs;
    if (outs[out].vals[0] > 0.0) {
        //ath_pin_high(&outs[out].pin);
        ath_pin_set(&outs[out].pin, ATH_HIGH);
    }
}

void athout_dc(uint8_t out, double dc) {
    if (outs[out].mode != PWM) return;

    ath_pin_pwm(&outs[out].pin, dc);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void outinit(out * o, uint8_t mode) {
    o->mode   = mode;
    o->state  = OFF;
    o->tstate = OFF;
    o->dt     = 0.0;
    o->i      = 1;
    o->f      = 0.0;
    o->t      = 0.0;
    o->vals   = NULL;
    o->vsize  = 0;
}
void outupdate(out * o, double dt) {
    if (o->state == ON) {
        if (!o->i) {
            //ath_pin_high(&o->pin);
            ath_pin_set(&o->pin, ATH_HIGH);
            o->i = 1;
        }
    } else
    if (o->state == OFF) {
        if (!o->i) {
            //ath_pin_low(&o->pin);
            ath_pin_set(&o->pin, ATH_LOW);
            o->i = 1;
        }
    } else
    if (o->state == BLINK) {
        o->dt += dt;
        
        if (o->tstate != BLINK) {
            o->t -= dt;
            if (o->t <= 0.0) {          /* stop animation */
                o->state = o->tstate;
                o->i = 0;
                return;
            }
        }
        
        if (o->dt >= o->f) {
            o->i ^= 1;
            o->dt -= o->f;
                                        /* do animation */
            if (o->i) ath_pin_set(&o->pin, ATH_HIGH);//ath_pin_high(&o->pin);
            else      ath_pin_set(&o->pin, ATH_LOW);//ath_pin_low(&o->pin);
        }

    
    } else
    if (o->state == SEQUENCE) {
        o->dt += dt;

        if (o->dt > o->vals[o->i]) {    /* increment i */
            o->i++;
            o->dt -= o->vals[o->i - 1];

            if (o->i >= o->vsize) {     /* stop or repeat animation */
                o->state = o->tstate;
                o->i = 0;
                if (o->vals[0] > 0.0) {
                    //ath_pin_high(&o->pin);
                    ath_pin_set(&o->pin, ATH_HIGH);
                }
                return;
            }
                                        /* do animation */
            if (o->i & 1) ath_pin_set(&o->pin, ATH_LOW);//ath_pin_low(&o->pin);
            else          ath_pin_set(&o->pin, ATH_HIGH);//ath_pin_high(&o->pin);

            
        }
    }

}

void outupdate_pwm(out * o, double dt) {

}










