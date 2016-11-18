#include "ath.h"

/* TODO
    - use new pin API
    - some noise reduction on speed computation
*/

#define FREQUENCY       30.0

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct decoder {
    uint8_t         xy;

    uint32_t        ppr;        /* pulses per revolution */
    uint32_t        decoded;
    int32_t         diff;

    double          position;
    double          rps;
} decoder;

void        computevalues(decoder * d, uint32_t now, double dt);

void        dec_init_d(decoder ** d, uint32_t ppra, uint32_t pprb);
void        dec_reset_d(decoder * d);
void        dec_setmult_d(uint8_t m);
void        dec_read_d(decoder * d, double dt);


//void        dec_read_s(decoder * d, double dt);


decoder     deca     = {0},
            decb     = {0};
decoder *   decs[2]  = {&deca, &decb};

ATH_UPL_DECLARE(wait);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athdecoder_init() {
    /* init the decoder */
    dec_init_d(decs,
        (ATHDECODER_DOUBLE_PPR * ATHDECODER_DOUBLE_MULT),
        (ATHDECODER_DOUBLE_PPR * ATHDECODER_DOUBLE_MULT));
}

void athdecoder_update(double dt) {
    ATH_UPL_CHECK(wait, FREQUENCY);

    dec_read_d(decs[ATHD_SIDEA], dt);
    dec_read_d(decs[ATHD_SIDEB], dt);

    ATH_UPL_RESET(wait);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athdecoder_reset(uint8_t side) {
    dec_reset_d(decs[side]);
}

/* absolute */
int32_t athdecoder_read(uint8_t side) {
    return (int32_t) decs[side]->decoded;
}
int32_t athdecoder_diff(uint8_t side) {
    return (int32_t) decs[side]->diff;
}

/* normalized */
double athdecoder_position(uint8_t side) {
    return decs[side]->position;
}
double athdecoder_rps(uint8_t side) {
    return decs[side]->rps;
}

double * athdecoder_getposition(uint8_t side) {
    return &(decs[side]->position);
}

double * athdecoder_getrps(uint8_t side) {
    return &(decs[side]->rps);
}
 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void computevalues(decoder * d, uint32_t now, double dt) {
    uint32_t dabs = abs(((int32_t) now) - ((int32_t) d->decoded));

    if (dabs < 100 && d->diff < 100) {
        now = d->decoded;
    }

    uint32_t old = d->decoded;
    d->decoded = now;

    /* compute diff */
    d->diff = d->decoded - old;

    /* update relative values */
    d->position = ((int32_t) d->decoded) / (double) d->ppr;
    d->rps      = (((int32_t) d->diff) / dt) / (double) d->ppr;
}

/* * * * * * * * * * * * * * * * *     HCTL2032      * * * * * * * * * * * * */
#define NONE_D              (BITMASK(ATHDECODER_OEN_PIN))
#define MSB1_D              (BITMASK(ATHDECODER_SEL2_PIN))
#define MSB2_D              (BITMASK(ATHDECODER_SEL1_PIN) |\
                                BITMASK(ATHDECODER_SEL2_PIN))
#define MSB3_D              (0x00)
#define MSB4_D              (BITMASK(ATHDECODER_SEL1_PIN))
#define WAIT_D              10
#define RESET_DELAY_D       10
void dec_init_d(decoder ** d, uint32_t ppra, uint32_t pprb) {
    /* set pins direction */
    GDDR(ATHDECODER_BYTE_PIN) = 0x00;
    PIN_DOUT(ATHDECODER_EN1_PIN);
    PIN_DOUT(ATHDECODER_EN2_PIN);
    PIN_DOUT(ATHDECODER_XY_PIN);
    GDDR(ATHDECODER_OEN_PIN) |= (BITMASK(ATHDECODER_OEN_PIN) |
        BITMASK(ATHDECODER_SEL1_PIN) | BITMASK(ATHDECODER_SEL2_PIN));
    PIN_DOUT(ATHDECODER_RESETX_PIN);
    PIN_DOUT(ATHDECODER_RESETY_PIN);

    /* set multiplicator */
    dec_setmult_d(ATHDECODER_DOUBLE_MULT);

    /* set to normal counting state */
    GPORT(ATHDECODER_OEN_PIN) = (GPORT(ATHDECODER_OEN_PIN) &
        ~(BITMASK(ATHDECODER_OEN_PIN) | BITMASK(ATHDECODER_SEL1_PIN) |
        BITMASK(ATHDECODER_SEL2_PIN))) | NONE_D;

    /* set x and y decs */
    d[ATHDECODER_2032_SIDEX]->xy = 0;
    d[ATHDECODER_2032_SIDEY]->xy = 1;

    /* reset counters */
    dec_reset_d(d[0]);
    dec_reset_d(d[1]);

    /* set PPRs */
    d[0]->ppr = ppra;
    d[1]->ppr = pprb;

}

void dec_reset_d(decoder * d) {
    if (d->xy) {
        PIN_LOW(ATHDECODER_RESETX_PIN);
        _delay_us(RESET_DELAY_D);
        PIN_HIGH(ATHDECODER_RESETX_PIN);
    } else {
        PIN_LOW(ATHDECODER_RESETY_PIN);
        _delay_us(RESET_DELAY_D);
        PIN_HIGH(ATHDECODER_RESETY_PIN);
    }

    /* also reset the structure */
    d->decoded  = 0;
    d->diff     = 0;
    d->position = 0.0;
    d->rps      = 0.0;
}

void dec_setmult_d(uint8_t m) {
    if (m == 1) {
        PIN_HIGH(ATHDECODER_EN1_PIN);
        PIN_HIGH(ATHDECODER_EN2_PIN);
    } else
    if (m == 2) {
        PIN_LOW(ATHDECODER_EN1_PIN);
        PIN_HIGH(ATHDECODER_EN2_PIN);
    } else
    if (m == 4) {
        PIN_HIGH(ATHDECODER_EN1_PIN);
        PIN_LOW(ATHDECODER_EN2_PIN);
    }
}

void dec_read_d(decoder * d, double dt) {
    /* select xy axis */
    if (!d->xy) { /* x axis */
        PIN_HIGH(ATHDECODER_XY_PIN);
    } else {     /* y axis */
        PIN_LOW(ATHDECODER_XY_PIN);
    }

    uint32_t val = 0x00000000;
    uint8_t pb = GPORT(ATHDECODER_OEN_PIN) & ~(BITMASK(ATHDECODER_OEN_PIN) |
        BITMASK(ATHDECODER_SEL1_PIN) | BITMASK(ATHDECODER_SEL2_PIN));

    /* 1st most significant octet */
    GPORT(ATHDECODER_OEN_PIN) = pb | MSB1_D;
    _delay_us(WAIT_D); /* Toen */
    val   = GPIN(ATHDECODER_BYTE_PIN);
    val <<= 8;

    /* 2nd most significant octet */
    GPORT(ATHDECODER_OEN_PIN) = pb | MSB2_D;
    _delay_us(WAIT_D);
    val  |= GPIN(ATHDECODER_BYTE_PIN);
    val <<= 8;

    /* 3rd most significant octet */
    GPORT(ATHDECODER_OEN_PIN) = pb | MSB3_D;
    _delay_us(WAIT_D);
    val  |= GPIN(ATHDECODER_BYTE_PIN);
    val <<= 8;

    /* 4th most significant octet */
    GPORT(ATHDECODER_OEN_PIN) = pb | MSB4_D;
    _delay_us(WAIT_D);
    val  |= GPIN(ATHDECODER_BYTE_PIN);

    /* exit */
    GPORT(ATHDECODER_OEN_PIN) = pb | NONE_D;

    if (!d->xy && ATHDECODER_2032_INVERTX) { /* x axis */
        val = (uint32_t) -((int32_t) val);
    }
    if (d->xy && ATHDECODER_2032_INVERTY) {  /* y axis */
        val = (uint32_t) -((int32_t) val);
    }

    /* update computed results */
    computevalues(d, val, dt);
}


/* * * * * * * * * * * * * * * * *      SINGLE       * * * * * * * * * * * * */
//void dec_init_s(decoder * d, uint32_t ppr) {
//    //ATHDECODER_BYTE_PIN
//    //ATHDECODER_SSO_PIN
//    //ATHDECODER_EN1_PIN
//    //ATHDECODER_EN2_PIN
//
//    //ATHDECODER_RESETX_PIN
//    //ATHDECODER_RESETY_PIN
//
//    //GDDR(ATHDECODER_BYTE_PIN) = 0x00;
//
//
//    //uint8_t  pb  = GPORT(ATHDECODER_SSO_PIN) & ((~0x07) << BIT(ATHDECODER_SSO_PIN));
//    //GPORT(ATHDECODER_SSO_PIN) = pb | (MSB1_S << BIT(ATHDECODER_SSO_PIN));
//
//
//    ///* selectors */
//    //PIN_DOUT(G0);
//    //PIN_DOUT(G1);
//    //PIN_DOUT(G2);
//
//    ///* reset */
//    //PIN_DOUT(D7);
//    //PIN_HIGH(D7);
//    ///*PIN_DIN(D7);
//    //PIN_HIGH(D7);*/
//
//    ///* byte reader */
//    //DDRC = 0x00;
//
//    ///* initial state */
//    //PORTG = SELECTOR | NONE;
//    //PIN_HIGH(D7);
//
//    //d->ppr = ppr;
//}
//
//
//void dec_reset_s(decoder * d) {
//
//}
//
//void dec_read_s(decoder * d, double dt) {
//    #define NONE_S            0x02
//    #define MSB1_S            0x04
//    #define MSB2_S            0x05
//    #define MSB3_S            0x00
//    #define MSB4_S            0x01
//    #define WAIT_S            1
//
//    uint32_t val = 0x00000000;
//    uint8_t  pb  = GPORT(ATHDECODER_SSO_PIN) & ((~0x07) << BIT(ATHDECODER_SSO_PIN));
//
//    /* 1st most significant octet */
//    GPORT(ATHDECODER_SSO_PIN) = pb | (MSB1_S << BIT(ATHDECODER_SSO_PIN));
//    _delay_us(WAIT_S); /* Toen */
//    val   = PINC;
//    val <<= 8;
//
//    /* 2nd most significant octet */
//    GPORT(ATHDECODER_SSO_PIN) = pb | (MSB2_S << BIT(ATHDECODER_SSO_PIN));
//    _delay_us(WAIT_S);
//    val  |= PINC;
//    val <<= 8;
//
//    /* 3rd most significant octet */
//    GPORT(ATHDECODER_SSO_PIN) = pb | (MSB3_S << BIT(ATHDECODER_SSO_PIN));
//    _delay_us(WAIT_S);
//    val  |= PINC;
//    val <<= 8;
//
//    /* 4th most significant octet */
//    GPORT(ATHDECODER_SSO_PIN) = pb | (MSB4_S << BIT(ATHDECODER_SSO_PIN));
//    _delay_us(WAIT_S);
//    val  |= PINC;
//
//    /* exit */
//    GPORT(ATHDECODER_SSO_PIN) = pb | (NONE << BIT(ATHDECODER_SSO_PIN));
//
//
//    computevalues(d, val, dt);
//}
//
//uint32_t readval(uint8_t xy) {
//    uint32_t val = 0x00000000;
//    uint8_t  pg  = SELECTOR;
//
//    /* 1st most significant octet */
//    PORTG = pg | MSB1;
//    _delay_us(WAIT); /* Toen */
//    val   = PINC;
//    val <<= 8;
//
//    /* 2nd most significant octet */
//    PORTG = pg | MSB2;
//    _delay_us(WAIT);
//    val  |= PINC;
//    val <<= 8;
//
//    /* 3rd most significant octet */
//    PORTG = pg | MSB3;
//    _delay_us(WAIT);
//    val  |= PINC;
//    val <<= 8;
//
//    /* 4th most significant octet */
//    PORTG = pg | MSB4;
//    _delay_us(WAIT);
//    val  |= PINC;
//
//    /* exit */
//    PORTG = pg | NONE;
//
//    return val;
//}
//
//
//
//
//
//

