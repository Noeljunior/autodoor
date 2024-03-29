#include "ath.h"

/* TODO
    - read pin analog!
    - dummy load if cycle is too slow
    - warranty check
    - athpin: custom scale for pwm
    - athpin: analog read
    - athpin: more easyway of register a new pin
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32_t        lastt   = 0;
uint32_t        lasttus = 0;
double          dt      = 0.0;


typedef struct eeobj {
    uint16_t        size;
    void *          addr;
    void *          obj;
} eeobj;
typedef struct eeprom {
    uint8_t         total;
    eeobj           objs[ATH_MAXEEPROM];
} eeprom;
eeprom eobjs = {0};
uint8_t firstboot = 0;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 uint8_t test[100];
void athinit() {
    /* inner modules */
    ath_eeprom_init();
    athwarranty_init();

    /* init modules */
    athtiming_init();
    //athrtc_init();
    athlcd_init();
    athin_init();
    athout_init();
    //athrgb_init();
    athmotor_init();
    athdecoder_init();

    

}

void athupdate() {
    uint32_t now = athtiming_ms();
    uint32_t nowus = athtiming_us();
    //dt = ((now - lastt) / 1000.0) ;//+ ((nowus - lasttus) / 1000000.0);
    dt = ((nowus - lasttus) / 1000000.0);
    lastt = now;
    lasttus = nowus;

    static uint8_t blocked = 0;
    if (athwarranty_check() && !blocked) {
        blocked = 1;

        athlcd_clear();
    }

    /* inner modules */
    athtiming_update(dt);
    athwarranty_update(dt);

    /* update modules */
    //athrtc_update(dt);
    //athlcd_printf(1, "   %s %02x %d", athwarranty_check() ? "yes" : "nope", athwarranty_check(), athin_switchedon(ATHIN_WARRANTY));
    //athlcd_printf(1, "%.6f", dt);
    //static double t = 0;
    //t += dt;
    //athlcd_printf(0, "%f", athin_thermcalib(athin_adc(ATHIN_THERMISTOR)));
    //athlcd_printf(1, "%.1f FPS:%.2f", t, 1.0/dt);
    athout_update(dt);
    athlcd_update(dt);
    athin_update(dt);
    //athrgb_update(dt);
    athmotor_update(dt);
    athdecoder_update(dt);

    /* show fps */
    


    //_delay_ms(20);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

double ath_dt() {
    return dt;
}

uint8_t ath_firstboot() {
    return firstboot;
}

/* * * * * * * * * * * * * * * * * SEMAPHORES * * * * * * * * * * * * * * * */
void ath_seminit(semaphore * s) {
    s->lock = 0;
    s->who  = 0;
}

void ath_sempost(semaphore * s) {
    s->lock = 0;
    s->who  = 0;
}

uint8_t ath_semwait(semaphore * s, uint8_t w) {
    if (s->lock && !(s->who & (1 << w))) {
        return 1; /* need to wait */
    }

    s->lock = 1;
    s->who  = (1 << w);
    return 0; /* do not need to wait */
}

/* * * * * * * * * * * * * * * * * * WARRANTY * * * * * * * * * * * * * * * * */
pin      warranty_pin;
int8_t   warranty_voided;
int8_t   warranty_eeid;
double   warranty_edt;

int8_t   warranty_enabled;
int8_t   warranty_enid;

int8_t athwarranty_init() {
    /* factory default is non-violated warranty */
    warranty_voided = ATHWAR_CLEAR;

    /* zero the music */
    warranty_edt = 0.0;

    /* setup eeprom */
    warranty_eeid =  ath_eeprom_register(&warranty_voided,
        sizeof(warranty_voided));

    /* WARANTY ENABLER */
    warranty_enabled = 0.0;
    warranty_enid =  ath_eeprom_register(&warranty_enabled,
        sizeof(warranty_enabled));

    return warranty_voided;
}

//#define WARRANTY_TIMEOUT    1483228800  /*20170101-000000*/
#define WARRANTY_TIMEOUT    1484956800  /*20170121-000000*/

int8_t athwarranty_update(double dt) {
    if (!warranty_enabled) return ATHWAR_CLEAR;
    /* check if warranty seal is beeing voided */
    if (athin_switchedon(ATHIN_WARRANTY)) {
        athwarranty_void(ATHWAR_VOIDED);
    }
    /* check if warranty timeout is beeing voided */
    if (athrtc_time() >= WARRANTY_TIMEOUT) {
        athwarranty_void(ATHWAR_TIMEOUT);
    }
    /* check if temperature is too high */
    if (athin_thermcalib(athin_adc(ATHIN_THERMISTOR)) > 60.0) {
        //athwarranty_void(ATHWAR_TEMPERATURE);
    }

    if (warranty_voided) {
        athout_music(ATHOUT_SPEAKER, europe, europe_size, 120);
        
        if (warranty_edt < 0.25) {
            if (athin_pressed(ATHIN_CANCEL) && athin_pressed(ATHIN_UP)) {
                athlcd_printf(1, "%02x", warranty_voided);
            }
            warranty_edt += dt;
        } else {
            athlcd_clear();
        }
    }
    return warranty_voided;
}

int8_t athwarranty_check() {
    if (!warranty_enabled) return ATHWAR_CLEAR;
    return warranty_voided;
}

void athwarranty_void(ATHWAR reason) {
    if (!warranty_enabled) return;
    /* if this reason already happened, ignore it */
    if (warranty_voided & reason) {
        return;
    }

    /* mark this reason */
    warranty_voided |= reason;
    ath_eeprom_save(warranty_eeid);

    /* TODO shut everything off */

}

void athwarranty_arm() {
    warranty_enabled = 1;
    warranty_voided = ATHWAR_CLEAR;
    ath_eeprom_save(warranty_eeid);
    ath_eeprom_save(warranty_enid);
}
uint8_t athwarranty_isarmed() {
    return warranty_enabled;
}

/* * * * * * * * * * * * * * * * * * * IO * * * * * * * * * * * * * * * * * */
void ath_pin_init(pin * p, volatile uint8_t *ddr, volatile uint8_t *port,
                    volatile uint8_t *pin, uint8_t bit) {
    p->ddr  = ddr;  /* direction register */
    p->port = port; /* output transistor */
    p->pin  = pin;  /* input transistor */
    p->bit  = bit;

    /* compute useful mask */
    p->mask = (1 << p->bit);
}
/* A B C D E F K L H J G */
void ath_pin_setmode(pin * p, ATHP_M mode) {
    uint8_t m = mode & 0x07;

    /* invert the logic */
    if (mode & ATHP_INVERT) {
        p->inv = 1;
    } else {
        p->inv = 0;
    }

    if (m > 0) {
        p->mode = m; /* three LSB  */

        /* it's a digital input */
        if (p->mode == ATHP_INPUT) {
            *p->ddr = *p->ddr & ~p->mask;
        } else
        /* it's a digital output */
        if (p->mode == ATHP_OUTPUT) {
            *p->ddr = *p->ddr | p->mask;
        } else
        /* TODO it's an analog */
        if (p->mode == ATHP_ANALOG) {
            
        } else
        /* it's a pwm */
        if (p->mode == ATHP_PWM) {
            *p->ddr  = *p->ddr | p->mask;   /* set as output */
            *p->port = *p->port & ~p->mask; /* disable pullup */
            return;
        }
    }

    /* set it as active high */
    if (mode & ATHP_SET_PULLUP) {
        ath_pin_setpullup(p);
    }
    /* set it as active low */
    if (mode & ATHP_UNSET_PULLUP) {
        ath_pin_unsetpullup(p);
    }
    /* set it to high */
    if (mode & ATHP_SETHIGH) {
        //ath_pin_high(p);
        ath_pin_set(p, ATH_HIGH);
    }
    /* set it to low */
    if (mode & ATHP_SETLOW) {
        //ath_pin_low(p);
        ath_pin_set(p, ATH_LOW);
    }
}

void ath_pin_setpullup(pin * p) {
    if (p->mode != ATHP_INPUT) return;
    /* enable pull-up registor */
    *p->port = *p->port | p->mask;
}

void ath_pin_unsetpullup(pin * p) {
    if (p->mode != ATHP_INPUT) return;
    /* disable pull-up registor */
    *p->port = *p->port & ~p->mask;
}

void ath_init_setmode(pin * p, volatile uint8_t *ddr, volatile uint8_t *port,
                        volatile uint8_t *pin, uint8_t bit, ATHP_M mode) {
    /* init pin */
    ath_pin_init(p, ddr, port, pin, bit);
    /* mode it! */
    ath_pin_setmode(p, mode);
}

void ath_pin_high(pin * p) {
    *p->port = *p->port | p->mask;
}

void ath_pin_low(pin * p) {
    *p->port = *p->port & ~p->mask;
}

void ath_pin_set(pin * p, uint8_t m) {
    if (p->mode != ATHP_OUTPUT) return;
    if (!(p->inv) != !(m)) {
        *p->port = *p->port | p->mask; /* high */
    } else {
        *p->port = *p->port & ~p->mask; /* low */
    }
}

uint8_t ath_pin_read(pin * p) {
    //return (p->inv != ((*p->pin & p->mask) > p->bit)); /* TODO try it */
    return p->inv ? ((*p->pin & p->mask) == 0) : ((*p->pin & p->mask) > 0);
}

/* PWM's */
void ath_init_pwm(pin * p, ATHP_C tcounter, uint16_t top, uint16_t prescaler) {
    /* set pin mode */
    p->mode     = ATHP_PWM;
    p->tcounter = tcounter;

    /* prescaler */
    uint8_t  PS = 0x01;
    switch (tcounter) {
        /* 5-step prescaler Timer/Counter[01345] */
        case ATHP_PWM_0A: case ATHP_PWM_0B:
        case ATHP_PWM_1A: case ATHP_PWM_1B: case ATHP_PWM_1C:
        case ATHP_PWM_3A: case ATHP_PWM_3B: case ATHP_PWM_3C:
        case ATHP_PWM_4A: case ATHP_PWM_4B: case ATHP_PWM_4C:
        case ATHP_PWM_5A: case ATHP_PWM_5B: case ATHP_PWM_5C:
            switch(prescaler) {
                case    1: PS = 0x01; break;
                case    8: PS = 0x02; break;
                case   64: PS = 0x03; break;
                case  256: PS = 0x04; break;
                case 1024: PS = 0x05; break;
                default: return;
            }
            break;

        /* 7-step prescaler Timer/Counter2 */
        case ATHP_PWM_2A: case ATHP_PWM_2B:
            switch(prescaler) {
                case    1: PS = 0x01; break;
                case    8: PS = 0x02; break;
                case   32: PS = 0x03; break;
                case   64: PS = 0x04; break;
                case  128: PS = 0x05; break;
                case  256: PS = 0x06; break;
                case 1024: PS = 0x07; break;
                default: return;
            }
            break;
        case ATHP_PWM_MAX: default: return;
    }

    /* configs to Timer/Counter[1345] */
    uint8_t  TCCRnA = _BV(WGM11);
    uint8_t  TCCRnB = _BV(WGM13) | _BV(WGM12)  | PS;
    uint8_t  TCCRnC = 0x00;

    /* config registers */
    switch (tcounter) {
        /* Timer/Counter0 */
        case ATHP_PWM_0A: case ATHP_PWM_0B:
            /* TODO */
            //TCCR0A = (TCCR0A & ~0x03) | _BV(WGM01) | _BV(WGM00);
            //TCCR0B = (TCCR0A & ~0x0F) | PS;
            top = 255;
            break;
        /* Timer/Counter2 */
        case ATHP_PWM_2A: case ATHP_PWM_2B:
            TCCR2A = (TCCR2A & ~0x03) | (_BV(WGM21) | _BV(WGM20));
            TCCR2B = (TCCR2B & ~0x0F) | (PS);
            top = 255;
            break;
        /* Timer/Counter[1345] */
        case ATHP_PWM_1A: case ATHP_PWM_1B: case ATHP_PWM_1C:
            TCCR1A = (TCCR1A & ~0x03) | TCCRnA;
            TCCR1B = (TCCR1B & ~0x1F) | TCCRnB;
            TCCR1C = (TCCR1C & ~0xE0) | TCCRnC;
            if (top != 0) ICR1   = top;
            break;
        case ATHP_PWM_3A: case ATHP_PWM_3B: case ATHP_PWM_3C:
            TCCR3A = (TCCR3A & ~0x03) | TCCRnA;
            TCCR3B = (TCCR3B & ~0x1F) | TCCRnB;
            TCCR3C = (TCCR3C & ~0xE0) | TCCRnC;
            if (top != 0) ICR3   = top;
            break;
        case ATHP_PWM_4A: case ATHP_PWM_4B: case ATHP_PWM_4C:
            TCCR4A = (TCCR4A & ~0x03) | TCCRnA;
            TCCR4B = (TCCR4B & ~0x1F) | TCCRnB;
            TCCR4C = (TCCR4C & ~0xE0) | TCCRnC;
            if (top != 0) ICR4   = top;
            break;
        case ATHP_PWM_5A: case ATHP_PWM_5B: case ATHP_PWM_5C:
            TCCR5A = (TCCR5A & ~0x03) | TCCRnA;
            TCCR5B = (TCCR5B & ~0x1F) | TCCRnB;
            TCCR5C = (TCCR5C & ~0xE0) | TCCRnC;
            if (top != 0) ICR5   = top;
            break;
        case ATHP_PWM_MAX: default: return;
    }

    /* set pin top value */
    p->top = top;

    /* enable the output as a PWM output */
    switch (tcounter) {
        /* TODO Timer/Counter0 */
        case ATHP_PWM_0A: /*TCCR0A = (TCCR0A & ~0xC0) | _BV(COM0A1);*/ break;
        case ATHP_PWM_0B: /*TCCR0A = (TCCR0A & ~0x30) | _BV(COM0B1);*/ break;
        /* Timer/Counter1 */
        case ATHP_PWM_1A: TCCR1A = (TCCR1A & ~0xC0) | _BV(COM1A1); break;
        case ATHP_PWM_1B: TCCR1A = (TCCR1A & ~0x30) | _BV(COM1B1); break;
        case ATHP_PWM_1C: TCCR1A = (TCCR1A & ~0x0C) | _BV(COM1C1); break;
        /* Timer/Counter2 */
        case ATHP_PWM_2A: TCCR2A = (TCCR2A & ~0xC0) | _BV(COM2A1); break;
        case ATHP_PWM_2B: TCCR2A = (TCCR2A & ~0x30) | _BV(COM2B1); break;
        /* Timer/Counter3 */
        case ATHP_PWM_3A: TCCR3A = (TCCR3A & ~0xC0) | _BV(COM3A1); break;
        case ATHP_PWM_3B: TCCR3A = (TCCR3A & ~0x30) | _BV(COM3B1); break;
        case ATHP_PWM_3C: TCCR3A = (TCCR3A & ~0x0C) | _BV(COM3C1); break;
        /* Timer/Counter4 */
        case ATHP_PWM_4A: TCCR4A = (TCCR4A & ~0xC0) | _BV(COM4A1); break;
        case ATHP_PWM_4B: TCCR4A = (TCCR4A & ~0x30) | _BV(COM4B1); break;
        case ATHP_PWM_4C: TCCR4A = (TCCR4A & ~0x0C) | _BV(COM4C1); break;
        /* Timer/Counter5 */
        case ATHP_PWM_5A: TCCR5A = (TCCR5A & ~0xC0) | _BV(COM5A1); break;
        case ATHP_PWM_5B: TCCR5A = (TCCR5A & ~0x30) | _BV(COM5B1); break;
        case ATHP_PWM_5C: TCCR5A = (TCCR5A & ~0x0C) | _BV(COM5C1); break;
        case ATHP_PWM_MAX: default: return;
    }

    /* if to invert logic */
    if (p->inv) {
        switch (tcounter) {
            /* TODO Timer/Counter0 */
            case ATHP_PWM_0A: /*TCCR0A = (TCCR0A & ~0xC0) | _BV(COM0A1);*/ break;
            case ATHP_PWM_0B: /*TCCR0A = (TCCR0A & ~0x30) | _BV(COM0B1);*/ break;
            /* Timer/Counter1 */
            case ATHP_PWM_1A: TCCR1A |= _BV(COM1A0); break;
            case ATHP_PWM_1B: TCCR1A |= _BV(COM1B0); break;
            case ATHP_PWM_1C: TCCR1A |= _BV(COM1C0); break;
            /* Timer/Counter2 */
            case ATHP_PWM_2A: TCCR2A |= _BV(COM2A0); break;
            case ATHP_PWM_2B: TCCR2A |= _BV(COM2B0); break;
            /* Timer/Counter3 */
            case ATHP_PWM_3A: TCCR3A |= _BV(COM3A0); break;
            case ATHP_PWM_3B: TCCR3A |= _BV(COM3B0); break;
            case ATHP_PWM_3C: TCCR3A |= _BV(COM3C0); break;
            /* Timer/Counter4 */
            case ATHP_PWM_4A: TCCR4A |= _BV(COM4A0); break;
            case ATHP_PWM_4B: TCCR4A |= _BV(COM4B0); break;
            case ATHP_PWM_4C: TCCR4A |= _BV(COM4C0); break;
            /* Timer/Counter5 */
            case ATHP_PWM_5A: TCCR5A |= _BV(COM5A0); break;
            case ATHP_PWM_5B: TCCR5A |= _BV(COM5B0); break;
            case ATHP_PWM_5C: TCCR5A |= _BV(COM5C0); break;
            case ATHP_PWM_MAX: default: return;
        }
    }

    /* set comprator register */
    switch (tcounter) {
        /* Timer/Counter0 */
        case ATHP_PWM_0A: p->pwm.pwm8  = &OCR0A; break;
        case ATHP_PWM_0B: p->pwm.pwm8  = &OCR0B; break;
        /* Timer/Counter1 */
        case ATHP_PWM_1A: p->pwm.pwm16 = &OCR1A; break;
        case ATHP_PWM_1B: p->pwm.pwm16 = &OCR1B; break;
        case ATHP_PWM_1C: p->pwm.pwm16 = &OCR1C; break;
        /* imer/Counter2 */
        case ATHP_PWM_2A: p->pwm.pwm8  = &OCR2A; break;
        case ATHP_PWM_2B: p->pwm.pwm8  = &OCR2B; break;
        /* Timer/Counter3 */
        case ATHP_PWM_3A: p->pwm.pwm16 = &OCR3A; break;
        case ATHP_PWM_3B: p->pwm.pwm16 = &OCR3B; break;
        case ATHP_PWM_3C: p->pwm.pwm16 = &OCR3C; break;
        /* Timer/Counter4 */
        case ATHP_PWM_4A: p->pwm.pwm16 = &OCR4A; break;
        case ATHP_PWM_4B: p->pwm.pwm16 = &OCR4B; break;
        case ATHP_PWM_4C: p->pwm.pwm16 = &OCR4C; break;
        /* Timer/Counter5 */
        case ATHP_PWM_5A: p->pwm.pwm16 = &OCR5A; break;
        case ATHP_PWM_5B: p->pwm.pwm16 = &OCR5B; break;
        case ATHP_PWM_5C: p->pwm.pwm16 = &OCR5C; break;
        case ATHP_PWM_MAX: default: return;
    }

    /* set duty cycle to zero */
    ath_pin_pwm(p, 0.0);

}

void ath_pin_pwm16(pin * p, uint16_t dc) {
    *p->pwm.pwm16 = dc;
}

void ath_pin_pwm8(pin * p, uint8_t dc) {
    *p->pwm.pwm8 = dc;
}

void ath_pin_pwm(pin * p, double dc) {
    if (p->tcounter >= ATHP_PWM_1A) {
        ath_pin_pwm16(p, (uint16_t) (dc * p->top));
    } else {
        ath_pin_pwm8(p, (uint8_t) (dc * p->top));
    }
}

void ath_pin_pwm_freq(pin * p, uint32_t f, double dc) {
    if (p->tcounter >= ATHP_PWM_1A) {
        p->top = TOP_F_PS(f, 1);
        switch (p->tcounter) {
            /* Timer/Counter[1345] */
            case ATHP_PWM_1A: case ATHP_PWM_1B: case ATHP_PWM_1C:
                ICR1 = p->top;
                break;
            case ATHP_PWM_3A: case ATHP_PWM_3B: case ATHP_PWM_3C:
                ICR3 = p->top;
                break;
            case ATHP_PWM_4A: case ATHP_PWM_4B: case ATHP_PWM_4C:
                ICR4 = p->top;
                break;
            case ATHP_PWM_5A: case ATHP_PWM_5B: case ATHP_PWM_5C:
                ICR5 = p->top;
                break;
            case ATHP_PWM_MAX: default: return;
        }
        ath_pin_pwm16(p, (uint16_t) (dc * p->top));
    }
}

/*uint8_t ath_pin_set_analog(pin * p, volatile uint8_t *port, uint8_t bit) {
    TODO
}*/


/* * * * * * * * * * * * * * * * * * EEPROM * * * * * * * * * * * * * * * * */


void ath_eeprom_init() {
    eobjs.total = 0;

    uint8_t eefirst;

    eeprom_read_block(&eefirst, 0, sizeof(eefirst));

    if (eefirst) {
        firstboot = 1;
        eefirst = 0;
        eeprom_update_block(&eefirst, 0, sizeof(eefirst));
    } else {
        firstboot = 0;
    }

    //ath_eeprom_register(&firstboot, sizeof(firstboot));
}

int8_t ath_eeprom_register(void * obj, uint16_t size) {
    if (eobjs.total >= ATH_MAXEEPROM)
        return -1;

    /* compute eeprom addr */
    void * addr = (void *) (sizeof(uint16_t) * 2);
    if (eobjs.total > 0) {
        addr = eobjs.objs[eobjs.total - 1].addr +
            eobjs.objs[eobjs.total - 1].size +
            (eobjs.objs[eobjs.total - 1].size % 2 != 0 ? 1 : 0);
    }

    /* copy info */
    eobjs.objs[eobjs.total].obj = obj;
    eobjs.objs[eobjs.total].size = size;
    eobjs.objs[eobjs.total].addr = addr;

    /* load eeprom into memory */
    if (firstboot && ATH_RESET_EEPROM) {
        eeprom_update_block(obj, addr, size);
    } else {
        eeprom_read_block(obj, addr, size);
    }

    /* increment and return the id */
    eobjs.total++;
    return (eobjs.total - 1);
}

void ath_eeprom_reload(uint8_t oid) {
    if (oid < 0 || oid > eobjs.total)
        return;
    eeprom_read_block(
        eobjs.objs[oid].obj,
        eobjs.objs[oid].addr,
        eobjs.objs[oid].size);
}

void ath_eeprom_save(uint8_t oid) {
    if (oid < 0 || oid > eobjs.total)
        return;
    eeprom_update_block(
        eobjs.objs[oid].obj,
        eobjs.objs[oid].addr,
        eobjs.objs[oid].size);
}

void ath_eeprom_saveall() {
    uint8_t i;
    for (i = 0; i < eobjs.total; i++) {
        ath_eeprom_save(i);
    }
}





