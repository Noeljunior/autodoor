#include "ath.h"

/* TODO
    - auto mode or manual/precise/mantainance mode
    - paper limiting
    - configurable direction inverter per motor
    - configurable speed atenuation per direction
    - if braking after slow walking, it gets a peak of pwm
    - better paper strength techinique
    - better targeting techinique
    - return state of motors

    no inicio, contar distancia total, dividir por estimativa e estimar
    quantos papeis estão instalados
    usar o sensor irda para esticar folha
 */



#define     PWM_HZ          20000UL
#define     LIMITTHRESHOLD  1000.0

#define     STRENGTH_DIST   ((1.0 - ATHMOTOR_SPEED_START) * \
                            ATHMOTOR_STRENGTH_PERC)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef enum STATE { UP, DOWN, STOP, BRAKE } STATE;
typedef enum MODE { M_NONE, M_ONESHOT, M_STICKY, M_FREE } MODE;
typedef struct motor {
    /* pins */
    pin         pspeed;
    pin         pbreak;
    pin         pdirection;
    pin         pfault;
    pin         ppwm;
    uint8_t     dirhight;
} motor;
typedef struct controler {
    motor       mup;
    motor       mdown;
    STATE       state;
    STATE       tstate;
    MODE        mode;
    double      speed;
    double      tspeed;
    double      speedf;
    //double      tspeedf;

    ATHMOTORO   which;

    double      target;
    uint8_t     targeted;
            /* paper limits : +/-LIMITTHRESHOLD is no limit defined */
    double      limit_start;
    double      limit_end;

    double      wait1;
            /* decoder values */
    double *    dposition;
    double *    drps;
} controler;

void        initcontrolor(controler * c);
void        controler_update(double dt, controler * c);

controler   controla,
            controlb;
controler*  controlers[] = { &controla, &controlb };


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athmotor_init() {
    /*
     *      SIDE A
     */

    /* UP */
    ath_init_setmode(&controla.mup.pspeed, GALL(ATHMOTOR_AUP_SPEED_PIN),
        ATHP_OUTPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mup.pbreak, GALL(ATHMOTOR_AUP_BRAKE_PIN),
        ATHP_OUTPUT | ATHP_SETHIGH);

    ath_init_setmode(&controla.mup.pdirection, GALL(ATHMOTOR_AUP_DIR_PIN),
        ATHP_OUTPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mup.pfault, GALL(ATHMOTOR_AUP_FAULT_PIN),
        ATHP_INPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mup.ppwm, GALL(ATHMOTOR_AUP_PWM_PIN),
        ATHP_OUTPUT);
    ath_init_pwm(&controla.mup.ppwm, ATHMOTOR_AUP_PWM_PWM, TOP_F_PS(PWM_HZ, 1), 1);

    /* DOWN */
    ath_init_setmode(&controla.mdown.pspeed, GALL(ATHMOTOR_ADOWN_SPEED_PIN),
        ATHP_OUTPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mdown.pbreak, GALL(ATHMOTOR_ADOWN_BRAKE_PIN),
        ATHP_OUTPUT | ATHP_SETHIGH);

    ath_init_setmode(&controla.mdown.pdirection, GALL(ATHMOTOR_ADOWN_DIR_PIN),
        ATHP_OUTPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mdown.pfault, GALL(ATHMOTOR_AUP_FAULT_PIN),
        ATHP_INPUT | ATHP_SETLOW);

    ath_init_setmode(&controla.mdown.ppwm, GALL(ATHMOTOR_ADOWN_PWM_PIN),
        ATHP_OUTPUT);
    ath_init_pwm(&controla.mdown.ppwm, ATHMOTOR_ADOWN_PWM_PWM, TOP_F_PS(PWM_HZ, 1), 1);

    initcontrolor(controlers[ATHM_SIDEA]);

    /* map position and rps to the controler */
    controlers[ATHM_SIDEA]->dposition = athdecoder_getposition(ATHD_SIDEA);
    controlers[ATHM_SIDEA]->drps      = athdecoder_getrps(ATHD_SIDEA);

    /*
     *      SIDE B
     */
    // TODO
    /* UP */
    /* DOWN */


}

void athmotor_update(double dt) {
    controler_update(dt, controlers[ATHM_SIDEA]);
    //controler_update(dt, controlers[ATHM_SIDEB]);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void athmotor_gos(uint8_t side, uint8_t wherehow, double speedfactor) {
    uint8_t where = wherehow & 0x03;
    uint8_t how   = wherehow & 0x0C;



    if (where == ATHM_UP) {
        controlers[side]->tstate = UP;
        controlers[side]->tspeed = 1.0;
        controlers[side]->speedf = speedfactor * ATHMOTOR_SPEED_FACTOR;
        controlers[side]->mode   = M_NONE;
    } else
    if (where == ATHM_DOWN) {
        controlers[side]->tstate = DOWN;
        controlers[side]->tspeed = -1.0;
        controlers[side]->speedf = speedfactor * ATHMOTOR_SPEED_FACTOR;
        controlers[side]->mode   = M_NONE;
    } else
    if (where == ATHM_STOP) {
        controlers[side]->tstate = STOP;
        controlers[side]->tspeed = 0.0;
        //controlers[side]->speedf = ATHM_NORMAL * ATHMOTOR_SPEED_FACTOR;
        controlers[side]->mode   = M_NONE;
    } else
    if (where == ATHM_BRAKE) {
        controlers[side]->tstate = BRAKE;
        controlers[side]->tspeed = 0.0;
        //controlers[side]->speedf = ATHM_NORMAL * ATHMOTOR_SPEED_FACTOR;
        controlers[side]->mode   = M_NONE;
    } else {
        return;
    }

    if (how != ATHM_SMOOTH) {
        controlers[side]->speed = controlers[side]->tspeed;
    }
}

void athmotor_go(uint8_t side, uint8_t wherehow) {
    athmotor_gos(side, wherehow, ATHM_NORMAL);
}

void athmotor_gotos(uint8_t side, double towhere, uint8_t sticky, double speedfactor) {
    controlers[side]->target = towhere;
    controlers[side]->speedf = speedfactor * ATHMOTOR_SPEED_FACTOR;
    controlers[side]->mode   = sticky == ATHM_STICKY ? M_STICKY : M_ONESHOT;
    controlers[side]->which  = ATHM_BOTH;
}

void athmotor_goto(uint8_t side, double towhere, uint8_t sticky) {
    athmotor_gotos(side, towhere, sticky, ATHM_NORMAL);
}

void athmotor_which(uint8_t side, ATHMOTOR which) {
    controlers[side]->which = which;
}

double athmotor_position(uint8_t side) {
    return *controlers[side]->dposition;
}

double athmotor_rps(uint8_t side) {
    return *controlers[side]->drps;
}

uint8_t athmotor_targeted(uint8_t side) {
    return (controlers[side]->targeted > 0);
}

void athmotor_set_limits(uint8_t side, double start, double end) {
    controlers[side]->limit_start = start;
    controlers[side]->limit_end   = end;
}
void athmotor_unset_limits(uint8_t side) {
    controlers[side]->limit_start = -LIMITTHRESHOLD;
    controlers[side]->limit_end   =  LIMITTHRESHOLD;
}

uint8_t athmotor_islimited(uint8_t side) {
    return (controlers[side]->limit_start > -LIMITTHRESHOLD &&
            controlers[side]->limit_end   < LIMITTHRESHOLD);
}

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void initcontrolor(controler * c) {
    c->state  = c->tstate = STOP;
    c->speed  = c->tspeed = 0.0;
    c->speedf = ATHM_NORMAL * ATHMOTOR_SPEED_FACTOR;
    c->mode   = M_NONE;

    c->wait1  = 0.0;

    c->limit_start = -LIMITTHRESHOLD;
    c->limit_end   =  LIMITTHRESHOLD;
}

void controler_update(double dt, controler * c) {
    uint8_t dobreak = 0;
    /* targeting */
    if (c->mode == M_ONESHOT || c->mode == M_STICKY/*c->target > 0.0*/) { /* stop if near target */
        double dist = fabs(*c->dposition - c->target);

        /* TODO speed limit based on remaining distance */
        double speedlimit = 1.0;

        //if (dist < 0.1) {
        //    speedlimit = 0.0;
        //} else
        //if (dist < 1.0) {
        //    speedlimit = 0.1;
        //    if (c->speedf > ATHM_NORMAL)
        //        c->speedf = ATHM_NORMAL;
        //} else {
        //    speedlimit = 1.0;
        //}

        #define THRESHOLD 1.0
        #define THRESHOLD_DIFF 0.05
        if (dist < THRESHOLD) {
            double pdist = dist / THRESHOLD;
            //if (fabs(c->tspeed - pdist) > THRESHOLD_DIFF)
            speedlimit = ATHT_EXP(pdist, 0.01, 1.0);
            //speedlimit = pdist;
        }


        /* set direction */
        if (*c->dposition < c->target) { /* need to go up */
            c->tstate = UP;
            c->tspeed = speedlimit;
        } else
        if (*c->dposition > c->target) { /* need to go down */
            c->tstate = DOWN;
            c->tspeed = -speedlimit;
        }

        /* break oposite near stop */
        if (dist < 0.05) {
            //dobreak = 1;
        }

        /* absolute stop */
        c->targeted = 0;
        if (dist < 0.02) {
            c->state = c->tstate = BRAKE;
            c->speed = c->tspeed = 0.0;
            if (c->mode == M_ONESHOT) {
                c->mode = M_NONE;
            }
            c->targeted = 1;
        }
    }

    /* paper limiting */
    if ((*c->dposition - ATHMOTOR_LIMIT_DIST) < c->limit_start &&
            (c->tspeed < 0.0)) {
        double dist = *c->dposition - c->limit_start;

        c->tspeed = -ATHMOTOR_LIMIT_SPEED;

        if (dist < ATHMOTOR_LIMIT_STOP_DIST) {
            c->state = c->tstate = BRAKE;
            c->speed = c->tspeed = 0.0;
            c->mode  = M_NONE;
            //c->targeted = 1;
        }
    } else
    if ((*c->dposition + ATHMOTOR_LIMIT_DIST) > c->limit_end &&
            (c->tspeed > 0.0)) {
        double dist = c->limit_end - *c->dposition;

        c->tspeed = ATHMOTOR_LIMIT_SPEED;

        if (dist < ATHMOTOR_LIMIT_STOP_DIST) {
            c->state = c->tstate = BRAKE;
            c->speed = c->tspeed = 0.0;
            c->mode  = M_NONE;
            //c->targeted = 1;
        }
    }

    /* speed-springing */
    double deltad = c->tspeed - c->speed;
    uint16_t incs = (uint16_t) fabs(deltad / dt);
    if (incs <= 0) { /* target reached */
        /* sync target to actual */
        c->state = c->tstate;
        c->speed = c->tspeed;
    }
    else { /* increment to the right direction */
        double inc = deltad / incs;
        if (fabs(c->tspeed) > fabs(c->speed)) {
            inc *= ATHMOTOR_SPEED_ACCEL;
        } else {
            inc *= ATHMOTOR_SPEED_DEACEL;
        }

        c->speed += inc;
    }

    /* to walk or to not walk */
    if      (c->speed > 0.0) c->state = UP;
    else if (c->speed < 0.0) c->state = DOWN;
    else                     c->state = c->tstate;

    /* compute the pwm */
    double pwmda = fabs(ATHT_SIN(c->speed, ATHMOTOR_SPEED_START, c->speedf));
    double pwmdb = pwmda;

    /* TODO strength on low speeding */
    pwmdb = pwmda * ATHMOTOR_STRENGTH_PERC;
    if (fabs(c->speed) < ATHMOTOR_STRENGTH_PERC) {
        double diff = fabs(c->speed);
        
        //if (diff > 0.8) {
        //    diff = 0.8;
        //}
        //if (diff <= (ATHMOTOR_SPEED_START / 2.0)) {
        //    diff = 0.1;
        //}
        //if (diff <= 0.1) {
        //    //pwmdb = 0.5;
        //    //dobreak = 1;
        //}
        pwmdb = pwmda * diff;
    }

    /* TODO check if speed is OK */
    //if (fabs(athdecoder_rps()) == 0.0 && c->wait1 > 0.5)
    //if (athdecoder_speed() <= 0 && pwmd > ATHMOTOR_SPEED_START * 2 &&
    //    (c->state == UP || c->state == DOWN)) {
    //    pwmd = 0.0;
    //}

    //if (fabs(c->speed) > 0.0 && fabs(*c->drps) < 0.1) {
    //    if (c->wait1 < 2.0) c->wait1 += dt;
    //} else {
    //    if (c->wait1 > 0.0) c->wait1 -= dt;
    //}
    //if (c->wait1 > 1.0) {
    //    pwmda = pwmdb = 0.0;
    //    c->wait1 = 2.0;
    //}
    //athlcd_printf(1, "%d %d, %d", (c->speed > 0.0), (fabs(*c->drps) < 0.05), (c->speed > 0.0) && (fabs(*c->drps) < 0.05));

    




    //ath_printf(1, "pwm: %4u %4u", pwmia, pwmib);
    //double s = (fabs(c->speed - c->tspeed) / (1-ATHMOTOR_SPEED_START-c->speedf)) ;
    //ath_printf(1, "to stop: %4d", (int) (s * 1000.0));

    /* TODO vary D according to actual speed to stretch */
    /* TODO strength the paper when stoping */

    /* stop if asked */

    /* check for absolute maximum */
    if (pwmda > ATHMOTOR_SPEED_ABSMAX) pwmda = ATHMOTOR_SPEED_ABSMAX;
    if (pwmdb > ATHMOTOR_SPEED_ABSMAX) pwmdb = ATHMOTOR_SPEED_ABSMAX;

    if (pwmda < ATHMOTOR_SPEED_ABSMIN) pwmda = ATHMOTOR_SPEED_ABSMIN;
    if (pwmdb < ATHMOTOR_SPEED_ABSMIN) pwmdb = ATHMOTOR_SPEED_ABSMIN;

    /* disable some motors */
    if (c->which < 0) { /* disable UP */
        ath_pin_pwm(&c->mup.ppwm, ATHMOTOR_SPEED_STOP);
        ath_pin_set(&c->mup.pbreak, ATH_HIGH);
    }
    if (c->which > 0) { /* disable DOWN */
        ath_pin_pwm(&c->mdown.ppwm, ATHMOTOR_SPEED_STOP);
        ath_pin_set(&c->mdown.pbreak, ATH_HIGH);
    }

    if (c->state == UP) { /* UP */
        /* UP MOTOR */
        if (c->which >= 0) {
            ath_pin_pwm(&c->mup.ppwm, pwmda * ATHMOTOR_CALIB_UPF * ATHMOTOR_CALIB_MUP);
            ath_pin_set(&c->mup.pdirection, ATH_LOW);
            ath_pin_set(&c->mup.pbreak, ATH_HIGH);
        }

        /* DOWN MOTOR */
        if (c->which <= 0) {
            ath_pin_pwm(&c->mdown.ppwm, pwmdb * ATHMOTOR_CALIB_UPF * ATHMOTOR_CALIB_MDOWN);
            ath_pin_set(&c->mdown.pdirection, ATH_HIGH);
            if (dobreak) ath_pin_set(&c->mdown.pbreak, ATH_LOW);
            else         ath_pin_set(&c->mdown.pbreak, ATH_HIGH);
        }
    } else if (c->state == DOWN) { /* DOWN */
        /* UP MOTOR */
        if (c->which >= 0) {
            ath_pin_pwm(&c->mup.ppwm, pwmdb * ATHMOTOR_CALIB_DPF * ATHMOTOR_CALIB_MUP);
            ath_pin_set(&c->mup.pdirection, ATH_HIGH);
            if (dobreak) ath_pin_set(&c->mup.pbreak, ATH_LOW);
            else         ath_pin_set(&c->mup.pbreak, ATH_HIGH);
        }

        /* DOWN MOTOR */
        if (c->which <= 0) {
            ath_pin_pwm(&c->mdown.ppwm, pwmda * ATHMOTOR_CALIB_DPF * ATHMOTOR_CALIB_MDOWN);
            ath_pin_set(&c->mdown.pdirection, ATH_LOW);
            ath_pin_set(&c->mdown.pbreak, ATH_HIGH);
        }
    } else if (c->state == BRAKE) {
        c->speedf = ATHM_NORMAL;
        /* UP MOTOR */
        if (c->which >= 0) {
            ath_pin_pwm(&c->mup.ppwm,   ATHMOTOR_SPEED_BRAKE);
            ath_pin_set(&c->mup.pbreak, ATH_LOW);
        }
        /* DOWN MOTOR */
        if (c->which <= 0) {
            ath_pin_pwm(&c->mdown.ppwm, ATHMOTOR_SPEED_BRAKE);
            ath_pin_set(&c->mdown.pbreak, ATH_LOW);
        }
    } else { /* STOP */
        c->speedf = ATHM_NORMAL;
        /* UP MOTOR */
        ath_pin_pwm(&c->mup.ppwm,   ATHMOTOR_SPEED_STOP);
        ath_pin_set(&c->mup.pbreak, ATH_HIGH);
        /* DOWN MOTOR */
        ath_pin_pwm(&c->mdown.ppwm, ATHMOTOR_SPEED_STOP);
        ath_pin_set(&c->mdown.pbreak, ATH_HIGH);
    }
}


