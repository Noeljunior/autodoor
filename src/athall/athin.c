#include "ath.h"

/* TODO
    - read analog
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define MAXTIME         (ATHIN_LPT * 10)

/* READ STATES */
#define SNN             (0)         /* no press */
#define SR              (1 << 0)    /* released */
#define SSC             (1 << 1)    /* short clicked */
#define SSP             (1 << 2)    /* short pressed */
#define SLC             (1 << 3)    /* long clicked */
#define SLP             (1 << 4)    /* long pressed */
#define SLCR            (1 << 5)    /* long clicked repeated */
#define SRTR            (1 << 7)    /* reset until release */

/* OPERATION MODES */
#define DIGITAL         (0)         /* if it is a digital */
#define ANALOG          (1 << 0)    /* if it is an analog */
#define SWITCH          (1 << 1)    /* on/off button */

typedef struct {
    volatile uint8_t *port;
    pin         pin;
    uint8_t     state;
    double      pressing;
    uint8_t     mode;
} inbutton;
inbutton        btns[ATHIN_MAX] = {0};

void        update_in(double dt, inbutton * in);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void athin_init() {
    /* ATHIN_OK */
    btns[ATHIN_OK].mode    = DIGITAL;
    ath_init_setmode(&btns[ATHIN_OK].pin,
        &GDDR(ATHIN_OK_PIN), &GPORT(ATHIN_OK_PIN),
        &GPIN(ATHIN_OK_PIN), BIT(ATHIN_OK_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_CANCEL */
    btns[ATHIN_CANCEL].mode = DIGITAL;
    ath_init_setmode(&btns[ATHIN_CANCEL].pin,
        &GDDR(ATHIN_CANCEL_PIN), &GPORT(ATHIN_CANCEL_PIN),
        &GPIN(ATHIN_CANCEL_PIN), BIT(ATHIN_CANCEL_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_UP */
    btns[ATHIN_UP].mode = DIGITAL;
    ath_init_setmode(&btns[ATHIN_UP].pin,
        &GDDR(ATHIN_UP_PIN), &GPORT(ATHIN_UP_PIN),
        &GPIN(ATHIN_UP_PIN), BIT(ATHIN_UP_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_DOWN */
    btns[ATHIN_DOWN].mode = DIGITAL;
    ath_init_setmode(&btns[ATHIN_DOWN].pin,
        &GDDR(ATHIN_DOWN_PIN), &GPORT(ATHIN_DOWN_PIN),
        &GPIN(ATHIN_DOWN_PIN), BIT(ATHIN_DOWN_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_LEFT */
    btns[ATHIN_LEFT].mode  = DIGITAL;
    ath_init_setmode(&btns[ATHIN_LEFT].pin,
        &GDDR(ATHIN_LEFT_PIN), &GPORT(ATHIN_LEFT_PIN),
        &GPIN(ATHIN_LEFT_PIN), BIT(ATHIN_LEFT_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_RIGHT */
    btns[ATHIN_RIGHT].mode  = DIGITAL;
    ath_init_setmode(&btns[ATHIN_RIGHT].pin,
        &GDDR(ATHIN_RIGHT_PIN), &GPORT(ATHIN_RIGHT_PIN),
        &GPIN(ATHIN_RIGHT_PIN), BIT(ATHIN_RIGHT_PIN),
        ATHP_INPUT | ATHP_ACTIVELOW | ATHP_SETHIGH | ATHP_INVERT);

    /* ATHIN_DOORA */
    btns[ATHIN_DOORA].mode = SWITCH;
    ath_init_setmode(&btns[ATHIN_DOORA].pin,
        &GDDR(ATHIN_DOORA_PIN), &GPORT(ATHIN_DOORA_PIN),
        &GPIN(ATHIN_DOORA_PIN), BIT(ATHIN_DOORA_PIN),
        ATHP_INPUT | ATHP_ACTIVEHIGHT | ATHP_SETLOW);

    /* ATHIN_PAPERA */
    btns[ATHIN_PAPERA].mode = SWITCH;
    ath_init_setmode(&btns[ATHIN_PAPERA].pin,
        &GDDR(ATHIN_PAPERA_PIN), &GPORT(ATHIN_PAPERA_PIN),
        &GPIN(ATHIN_PAPERA_PIN), BIT(ATHIN_PAPERA_PIN),
        ATHP_INPUT | ATHP_ACTIVEHIGHT | ATHP_SETLOW);
}


void athin_update(double dt) {
    uint8_t i;

    for (i = 0; i < ATHIN_MAX; i++) {
        update_in(dt, btns + i);
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athin_reset(uint8_t in) {
    btns[in].state = SNN;
    btns[in].pressing = 0.0;
}
void athin_ignrel(uint8_t in) {
    btns[in].state = SNN;
    btns[in].pressing = 0.0;
}

uint8_t athin_clicked(uint8_t in) {
    return (btns[in].state == SSC);
}
uint8_t athin_longclicked(uint8_t in) {
    return (btns[in].state == SLC);
}
uint8_t athin_exclicked(uint8_t in) {
    return (btns[in].state == SR &&
            btns[in].pressing < ATHIN_LPT);
}
uint8_t athin_released(uint8_t in) {
    return (btns[in].state == SR);
}
uint8_t athin_clicking(uint8_t in) {
    return (btns[in].state == SSC ||
            btns[in].state == SLC ||
            btns[in].state == SLCR);
}
uint8_t athin_pressed(uint8_t in) {
    return (btns[in].state > SR);
}
uint8_t athin_longpressed(uint8_t in) {
    return (btns[in].state > SR &&
            btns[in].pressing > ATHIN_LPT);
}
uint8_t athin_switchedon(uint8_t in) {
    return (btns[in].state);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void update_in(double dt, inbutton * in) {
    uint8_t read;

    /* analog read */
    if (in->mode & (uint8_t) ANALOG) {
        //in->state = athgeta(keyboard->kbpin[i]);
        // TODO read analog
        return;
    }
    /* digital read */
    else {
        //read = (*in->port) & (1 << in->bit);
        read = ath_pin_read(&in->pin);
        // TODO invert signa : pull ups / active low

        /* a switch button with no proccessing */
        if ((int) in->mode & SWITCH) {
            in->state = read ? SSP : SNN;
            return;
        }

        /* button released */
        if (!read) {
            if ((in->state & ~SR) > SNN) {
                in->state = SR;
            } else {
                in->pressing = 0.0;
                in->state = SNN;
            }
            return;
        }

        /* pressing time */
        if (in->pressing < MAXTIME)
            in->pressing += dt;

        /* short clicked */
        if (in->state == SNN) {
            in->state = SSC;
            return;
        } else
        /* short pressed */
        if (((in->state == SSC) ||
             (in->state == SSP)) &&
             in->pressing < ATHIN_LPT) {
            in->state = SSP;
            return;
         } else
         /* long clicked */
         if (in->state == SSP &&
             in->pressing >= ATHIN_LPT) {
            in->state = SLC;
            return;
         } else
         /* long pressed */
         if ((in->state == SLC) ||
             (in->state == SLP) ||
             (in->state == SLCR)) {
            in->state = SLP;
            /* long clicked repeated */
            if (in->pressing >= (ATHIN_LPT + ATHIN_LPR)) {
                in->pressing -= ATHIN_LPR;
                in->state = SLCR;
            }
            return;
         } else
         /* some unknown state, reset it */
         {
            in->state = SNN;
            in->pressing = 0.0;
            return;
         }
    }
}
