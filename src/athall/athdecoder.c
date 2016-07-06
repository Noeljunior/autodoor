#include "ath.h"

/* TODO
    - use new pin API
    - some noise reduction on speed computation
*/

#define FREQUENCY       30.0
#define WAIT            1
#define SELECTOR        (PORTG & ~0x07)

#define NONE            0x02
#define MSB1            0x04
#define MSB2            0x05
#define MSB3            0x00
#define MSB4            0x01

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct decval {
    uint32_t        ppr;        /* pulses per revolution */
    uint32_t        decoded;
    int32_t         diff;

    double          position;
    double          rps;
} decval;

uint32_t    readval(uint8_t xy);
void        computevalues(decval * d, uint32_t now, double wait);

decval      decvala     = {0},
            decvalb     = {0};
decval *    decvals[2]  = {&decvala, &decvalb};

ATH_UPL_DECLARE(wait);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athdecoder_init() {
    /* selectors */
    PIN_DOUT(G0);
    PIN_DOUT(G1);
    PIN_DOUT(G2);

    /* reset */
    PIN_DOUT(D7);
    PIN_HIGH(D7);
    /*PIN_DIN(D7);
    PIN_HIGH(D7);*/

    /* byte reader */
    DDRC = 0x00;

    /* initial state */
    PORTG = SELECTOR | NONE;
    PIN_HIGH(D7);

    //athdecoder_reset();


    /* init decoders */
    decvals[ATHD_SIDEA]->ppr = ATHDECODER_PPRA;
    decvals[ATHD_SIDEB]->ppr = ATHDECODER_PPRB;

}

void athdecoder_update(double dt) {
    ATH_UPL_CHECK(wait, FREQUENCY);

    computevalues(decvals[ATHD_SIDEA], readval(0), wait);
    wait = 0.0;

    ATH_UPL_RESET(wait);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* absolute */
int32_t athdecoder_read(uint8_t side) {
    return (int32_t) decvals[side]->decoded;
}
int32_t athdecoder_diff(uint8_t side) {
    return (int32_t) decvals[side]->diff;
}

void athdecoder_reset(uint8_t side) {
    PIN_LOW(D7);
    _delay_us(50);
    PIN_HIGH(D7);
    _delay_us(10);

    /* also reset the struct */
    decvals[side]->decoded  = 0;
    decvals[side]->diff     = 0;
    decvals[side]->position = 0.0;
    decvals[side]->rps      = 0.0;
}

/* normalized */
double athdecoder_position(uint8_t side) {
    return decvals[side]->position;
}
double athdecoder_rps(uint8_t side) {
    return decvals[side]->rps;
}

double * athdecoder_getposition(uint8_t side) {
    return &(decvals[side]->position);
}

double * athdecoder_getrps(uint8_t side) {
    return &(decvals[side]->rps);
}
 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32_t readval(uint8_t xy) {
    uint32_t val = 0x00000000;
    uint8_t  pg  = SELECTOR;

    /* 1st most significant octet */
    PORTG = pg | MSB1;
    _delay_us(WAIT); /* Toen */
    val   = PINC;
    val <<= 8;

    /* 2nd most significant octet */
    PORTG = pg | MSB2;
    _delay_us(WAIT);
    val  |= PINC;
    val <<= 8;

    /* 3rd most significant octet */
    PORTG = pg | MSB3;
    _delay_us(WAIT);
    val  |= PINC;
    val <<= 8;

    /* 4th most significant octet */
    PORTG = pg | MSB4;
    _delay_us(WAIT);
    val  |= PINC;

    /* exit */
    PORTG = pg | NONE;

    return val;
}

void computevalues(decval * d, uint32_t now, double wait) {
    uint32_t old = d->decoded;
    d->decoded = now;

    /* compute diff */
    d->diff = d->decoded - old;

    /* update relative values */
    d->position = ((int32_t) d->decoded) / (double) d->ppr;
    d->rps      = (((int32_t) d->diff) / wait) / (double) d->ppr;
}




