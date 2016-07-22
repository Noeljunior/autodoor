#include "ath.h"

/* TODO
    - 
*/



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum    EFFECT {
                    FADE            = (1 << 0),
                    FLICKERING      = (1 << 1),
} EFFECT;

typedef struct {
    pin         pinr, ping, pinb;

    /* actual colour */
    double      ar, ag, ab;

    /* while calib */
    double      cr, cg, cb;

    /* ramp calib */
    double      rr, rg, rb;

    /* effects */
    EFFECT      etype;
    double      odt, edt;
    double      er, eg, eb;
    double      ef;

} rgb;
rgb             rgbs[ATHRGB_MAX] = {0};


void            rgb_init(rgb * p);
void            rgb_update(double dt, rgb * p);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athrgb_init() {
    /* ATHRGB_LEDR */
    ath_init_setmode(&rgbs[ATHRGB_P1].pinr, GALL(ATHRGB_LEDR_PIN),
        ATHP_OUTPUT | ATHP_INVERT);
    ath_init_pwm(&rgbs[ATHRGB_P1].pinr, ATHRGB_LEDR_PWM,
        TOP_F_PS(ATHRGB_PWM_FREQ_HZ, 1), 1);
    /* ATHRGB_LEDG */
    ath_init_setmode(&rgbs[ATHRGB_P1].ping, GALL(ATHRGB_LEDG_PIN),
        ATHP_OUTPUT | ATHP_INVERT);
    ath_init_pwm(&rgbs[ATHRGB_P1].ping, ATHRGB_LEDG_PWM,
        TOP_F_PS(ATHRGB_PWM_FREQ_HZ, 1), 1);
    /* ATHRGB_LEDB */
    ath_init_setmode(&rgbs[ATHRGB_P1].pinb, GALL(ATHRGB_LEDB_PIN),
        ATHP_OUTPUT | ATHP_INVERT);
    ath_init_pwm(&rgbs[ATHRGB_P1].pinb, ATHRGB_LEDB_PWM,
        TOP_F_PS(ATHRGB_PWM_FREQ_HZ, 1), 1);
    rgb_init(&rgbs[ATHRGB_P1]);
    athrgb_calib(ATHRGB_P1, 1.0, 1.0, 0.7);
    athrgb_ramp(ATHRGB_P1, 0.1, 0.8, 1.0);
}

void athrgb_update(double dt) {
    uint8_t i;
    for (i = 0; i < ATHRGB_MAX; i++) {
        rgb_update(dt, &rgbs[i]);
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athrgb_calib(ATHRGB rgb, double cr, double cg, double cb) {
    rgbs[rgb].cr = cr;
    rgbs[rgb].cg = cg;
    rgbs[rgb].cb = cb;
}

void athrgb_ramp(ATHRGB rgb, double rr, double rg, double rb) {
    rgbs[rgb].rr = rr;
    rgbs[rgb].rg = rg;
    rgbs[rgb].rb = rb;
}

void athrgb_rgb(ATHRGB rgb, double r, double g, double b) {
    rgbs[rgb].ar = r;
    rgbs[rgb].ag = g;
    rgbs[rgb].ab = b;
}

void athrgb_hsv(ATHRGB rgb, double h, double s, double v) {
    HSVtoRGB(h, s, v, &rgbs[rgb].ar);
}

void athrgb_hsl(ATHRGB rgb, double h, double s, double l) {
    HSLtoRGB(h, s, l, &rgbs[rgb].ar);
}

void athrgb_fadeto(ATHRGB rgb, double a, double b, double c, double dt, uint8_t t) {
    /* if in the midle of effect, correct values */
    if (rgbs[rgb].edt > 0) {
        double p1 = (rgbs[rgb].edt / rgbs[rgb].odt);
        double p0 = 1.0 - p1;
        
        rgbs[rgb].ar = rgbs[rgb].er * p0 + rgbs[rgb].ar * p1;
        rgbs[rgb].ag = rgbs[rgb].eg * p0 + rgbs[rgb].ag * p1;
        rgbs[rgb].ab = rgbs[rgb].eb * p0 + rgbs[rgb].ab * p1;
    }

    rgbs[rgb].ef    = 1.0;
    rgbs[rgb].edt   =
    rgbs[rgb].odt   = dt;


    if (t == ATHRGB_RGB) {
        rgbs[rgb].er = a;
        rgbs[rgb].eg = b;
        rgbs[rgb].eb = c;
    } else if (t == ATHRGB_HSV) {
        HSVtoRGB(a, b, c, &rgbs[rgb].er);
    } else if (t == ATHRGB_HSL) {
        HSLtoRGB(a, b, c, &rgbs[rgb].er);
    }
}

void athrgb_flicker_on(ATHRGB rgb) {
    rgbs[rgb].etype |= FLICKERING;
}
void athrgb_flicker_off(ATHRGB rgb) {
    rgbs[rgb].etype &= ~FLICKERING;
}


uint8_t athrgb_fading(ATHRGB rgb) {
    return rgbs[rgb].edt > 0.0;
}



void HSVtoRGB(double h, double s, double v, double *rgb) {
    h = fmod(h * 360.0, 360.0);
    double c = v * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));
    double m = v - c;
    if      (h < 60.0)  { rgb[0] = c + m; rgb[1] = x + m; rgb[2] = m;     }
    else if (h < 120.0) { rgb[0] = x + m; rgb[1] = c + m; rgb[2] = m;     }
    else if (h < 180.0) { rgb[0] = m;     rgb[1] = c + m; rgb[2] = x + m; }
    else if (h < 240.0) { rgb[0] = m;     rgb[1] = x + m; rgb[2] = c + m; }
    else if (h < 300.0) { rgb[0] = x + m; rgb[1] = m;     rgb[2] = c + m; }
    else if (h < 360.0) { rgb[0] = c + m; rgb[1] = m;     rgb[2] = x + m; }
    else                { rgb[0] = m;     rgb[1] = m;     rgb[2] = m;     }
}

void HSLtoRGB(double h, double s, double l, double *rgb) {
    h = fmod(h * 360.0, 360.0);
    double c = (1 - fabs(2 * l - 1)) * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));
    double m = l - c / 2.0;
    if      (h < 60.0)  { rgb[0] = c + m; rgb[1] = x + m; rgb[2] = m;     }
    else if (h < 120.0) { rgb[0] = x + m; rgb[1] = c + m; rgb[2] = m;     }
    else if (h < 180.0) { rgb[0] = m;     rgb[1] = c + m; rgb[2] = x + m; }
    else if (h < 240.0) { rgb[0] = m;     rgb[1] = x + m; rgb[2] = c + m; }
    else if (h < 300.0) { rgb[0] = x + m; rgb[1] = m;     rgb[2] = c + m; }
    else if (h < 360.0) { rgb[0] = c + m; rgb[1] = m;     rgb[2] = x + m; }
    else                { rgb[0] = m;     rgb[1] = m;     rgb[2] = m;     }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void rgb_init(rgb * p) {

}

double edt = 0.0;
void rgb_update(double dt, rgb * p) {
    double r = p->ar, g = p->ag, b = p->ab;

    /* do flicker */
    if (FLICKERING & p->etype) {
        if (random() < (RANDOM_MAX * 0.5)) {
            edt = (((double)random() / ((double)RANDOM_MAX + 1) * 0.05)+0.001);
            p->ef = 0.0;
        } else {
            edt = (((double)random() / ((double)RANDOM_MAX + 1) * 0.05)+0.001);
            p->ef = 1.0;
        }
    }

    /* do fade */
    if (p->edt > 0.0) {
        double p0 = 1.0 - (p->edt / p->odt);
        double p1 = (p->edt / p->odt);
        
        r = p->er * p0 + p->ar * p1;
        g = p->eg * p0 + p->ag * p1;
        b = p->eb * p0 + p->ab * p1;
        
        p->edt -= dt;
        if (p->edt <= 0.0) { /* reset effect */
            p->ar = p->er;
            p->ag = p->eg;
            p->ab = p->eb;
        }
    }
    r *= p->ef;
    g *= p->ef;
    b *= p->ef;


    ath_pin_pwm(&p->pinr, ATHT_EXP(r * p->cr, p->rr, 1));
    ath_pin_pwm(&p->ping, ATHT_EXP(g * p->cg, p->rg, 1));
    ath_pin_pwm(&p->pinb, ATHT_EXP(b * p->cb, p->rb, 1));

    //athlcd_printf(1, "%.2f %.2f %.2f",
        //l_hsl[0], l_hsl[1], l_hsl[2]);
        //l_dc[0], l_dc[1], l_dc[2]);
    //    ATHT_EXP(p->ar * p->cr, p->rr, 1), ATHT_EXP(p->ag * p->cg, p->rg, 1), ATHT_EXP(p->ab * p->cb, p->rb, 1));

}








