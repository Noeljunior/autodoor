#include "ats.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct syssettings {
    uint8_t     doublesided;
                /* motors settings */
    double      limit_start[ATH_SIDES];
    double      limit_end[ATH_SIDES];
    double      lenght[ATH_SIDES];

    atsp_target target[ATH_SIDES][ATSP_MAXTARGETS];
} syssettings;
syssettings settings;

/* state */
typedef struct pstate {
    ATSP_ASK        doing;

    uint8_t         side;
    uint8_t         door;
    uint8_t         paper;

    atsp_target *   trgs;

    /*
        TASK SPECIFIC
    */
    /* referencing */
    struct  aauto {
        double      wfd;
        double      wait;
        int8_t      dir;
        uint8_t     trg;
    } aauto;
    /* referencing */
    struct  aref {
        uint8_t     state;
        uint8_t     mismatch;
        double      safe_dist;
        double      wait;
        double      limit_start,
                    limit_end;
        double      lenght;
        atsp_target trgs[ATSP_MAXTARGETS];
    } aref;

} pstate;
pstate state[ATH_SIDES] = {0};
semaphore locksem;

void        init_panel(pstate * s, uint8_t side, uint8_t door, uint8_t paper);
void        update_panel(double dt, pstate * s);

void        reference_init(pstate * s);
uint8_t     reference(pstate * s, double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 SYS INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void atspanel_init() {
    /* init structure and state */
    init_panel(state + ATH_SIDEA, ATH_SIDEA, ATHIN_DOORA, ATHIN_PAPERA);
    //init_panel(state + ATH_SIDEB, ATH_SIDEB, ATHIN_DOORB, ATHIN_PAPERB);

    /* init semaphore */
    ath_seminit(&locksem);

    /* initalize motors state */


    /* postitions test */
    /**/settings.target[ATH_SIDEA][0].inuse    = 1;
    settings.target[ATH_SIDEA][0].target   = 0.73;
    settings.target[ATH_SIDEA][0].duration = 5.0;
    settings.target[ATH_SIDEA][3].inuse    = 1;
    settings.target[ATH_SIDEA][3].target   = 8.77;
    settings.target[ATH_SIDEA][3].duration = 5.0;
    settings.target[ATH_SIDEA][9].inuse    = 1;
    settings.target[ATH_SIDEA][9].target   = 16.84;
    settings.target[ATH_SIDEA][9].duration = 5.0;
    
    /*settings.target[ATH_SIDEA][10].inuse    = 1;
    settings.target[ATH_SIDEA][10].target   = 12.0;
    settings.target[ATH_SIDEA][10].duration = 1.0;
    settings.target[ATH_SIDEA][11].inuse    = 1;
    settings.target[ATH_SIDEA][11].target   = 12.0;
    settings.target[ATH_SIDEA][11].duration = 1.0;
    settings.target[ATH_SIDEA][12].inuse    = 1;
    settings.target[ATH_SIDEA][12].target   = 12.0;
    settings.target[ATH_SIDEA][12].duration = 1.0;*/

}

void atspanel_update(double dt) {
    /* TODO control both panel in mutual exclusion */


    update_panel(dt, &state[ATH_SIDEA]);
    //update_panel(dt, &state[ATH_SIDEB]);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void atspanel_ask(uint8_t side, uint8_t ask) {
    /* proccess needs */
    if (ask & ATSP_REFERENCE) { /* beeing asked to reference */
        /* TODO check for problems that might be imcompatible with referencing */
        if (state[side].doing & ATSP_REFERENCE) { /* if already doing it, drop it */
            //s->asking &= ~ATSP_REFERENCE;
            return;
        }

        /* init referecing */
        reference_init(&state[side]);

        /* update asks */
        //s->asking &= ~ATSP_REFERENCE;
        state[side].doing  |=  ATSP_REFERENCE;
        //athlcd_printf(1, "ASKING");
    }
    if (ask & ATSP_OFF) { /* beeing asked to go to off mode */

        /* reset states after this big asking */
        //s->asking = 0;
        state[side].doing  = ATSP_OFF;
    }
    if (ask & ATSP_SAUTO) { /* beeing asked to go auto mode */

        /* reset states after this big asking */
        //s->asking = 0;
        state[side].doing  = ATSP_SAUTO;
    }
    if (ask & ATSP_SMANUAL) { /* beeing asked to go auto mode */
        /* TODO check any inconvenience */

        /* reset env things we dunno if they changed */
        atspanel_inconsistent(side);

        athmotor_go(side, ATHM_BRAKE);

        /* reset states after this big asking */
        //s->asking = 0;
        state[side].doing  = ATSP_SMANUAL;
    }
}

uint8_t atspanel_isdoing(uint8_t side, uint8_t ask) {
    return (state[side].doing & ask);
}

uint8_t atspanel_mismatched(uint8_t side) {
    return state[side].aref.mismatch;
}

atsp_target * atspanel_getrefstmp(uint8_t side) {
    return state[side].aref.trgs;
}
atsp_target * atspanel_getrefs(uint8_t side) {
    return state[side].trgs;
}

uint8_t atspanel_counttrgs_active(atsp_target * ts) {
    uint8_t c = 0, i;
    for (i = 0; i < ATSP_MAXTARGETS; i++) {
        if (ts[i].inuse) c++;
    }
    return c;
}

uint8_t atspanel_counttrgs_useful(atsp_target * ts) {
    uint8_t c = 0, i;
    for (i = 0; i < ATSP_MAXTARGETS; i++) {
        if (ts[i].inuse && ts[i].duration >= 1.0) c++;
    }
    return c;
}

void atspanel_copytrgs(atsp_target * src, atsp_target * dst) {
    uint8_t i;
    for (i = 0; i < ATSP_MAXTARGETS; i++) {
        dst[i] = src[i];
    }
}

void atspanel_savetargets(uint8_t side) {
    uint8_t i;
    for (i = 0; i < ATSP_MAXTARGETS; i++) {
        state[side].trgs[i] = state[side].aref.trgs[i];
    }
}

void atspanel_walk(uint8_t side, uint8_t keyup, uint8_t keydown) {
    if (athin_clicked(keyup)) {
        athmotor_go(ats_wside(), ATHM_UP);
    } else
    if (athin_clicked(keydown)) {
        athmotor_go(ats_wside(), ATHM_DOWN);
    }
    if (athin_released(keyup) || athin_released(keydown)) {
        athmotor_go(ats_wside(), ATHM_BRAKE);
    }
}

void atspanel_hobble_up(uint8_t side, uint8_t keyup, uint8_t keydown) {

}

void atspanel_hobble_down(uint8_t side, uint8_t keyup, uint8_t keydown) {

}

void atspanel_free(uint8_t side) {
    athmotor_go(side, ATHM_STOP);
}

void atspanel_brake(uint8_t side) {
    athmotor_go(side, ATHM_BRAKE);
}

uint8_t atspanel_opened(uint8_t side) {
    return athin_switchedon(state[side].door);
}

uint8_t atspanel_torn(uint8_t side) {
    return athin_switchedon(state[side].paper);
}

void atspanel_inconsistent(uint8_t side) {
    /* unset motors limits */
    athmotor_unset_limits(side);

    /* stop what is doing */
    if (state[side].doing & ATSP_OFF) {
        state[side].doing = ATSP_OFF;
    } else
    if (state[side].doing & ATSP_SAUTO) {
        state[side].doing = ATSP_SAUTO;
    } else
    if (state[side].doing & ATSP_SMANUAL) {
        state[side].doing = ATSP_SMANUAL;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void init_panel(pstate * s, uint8_t side, uint8_t door, uint8_t paper) {
    s->doing = ATSP_OFF;
    //s->asking = ATSP_SAUTO;

    s->side  = side;
    s->door  = door;
    s->paper = paper;

    s->trgs  = settings.target[side];
}

uint8_t getnexti_greedy(atsp_target * t, uint8_t i, int8_t *dir) {
    uint8_t oi = i;
    while (i >= 0 && i < ATSP_MAXTARGETS) {
        i += *dir;
        if (t[i].inuse && t[i].duration >= 1.0) { /* found */
            return i;
        }
    }
    *dir *= -1;
    i = oi;
    while (i >= 0 && i < ATSP_MAXTARGETS) {
        i += *dir;
        if (t[i].inuse && t[i].duration >= 1.0) { /* found */
            return i;
        }
    }
    *dir *= -1;
    return oi;
}


void update_panel(double dt, pstate * s) {
    /* do what is suposed to be doing */
    if (s->doing & ATSP_OFF) { /* stop motors */
        athmotor_go(s->side, ATHM_STOP | ATHM_HARD);
    } else {
        if (s->doing & ATSP_SAUTO) { /* AUTO MODE */
            if (!(s->doing & ATSP_AREADY)) {
                /* check number of papers */
                if (atspanel_counttrgs_useful(s->trgs) < 1) {
                
                    athlcd_printf(1, "Sem folhas defs");
                    return;
                }
                /* if door is opnen... */
                if (athin_switchedon(s->door)) {
                    athmotor_go(s->side, ATHM_BRAKE);
                    s->aauto.wfd = 3.0;
                    athlcd_printf(1, "Porta aberta...");
                    atspanel_inconsistent(s->side);
                    return;
                } else
                if (s->aauto.wfd > 0.0) {
                    s->aauto.wfd -= dt;
                    athlcd_printf(1, "Iniciar em %s",
                        ats_time_tos(s->aauto.wfd, 1));
                    return;
                }
                /* do reference */
                if (s->doing & ATSP_REFERENCE) {
                    if (!reference(s, dt)) {
                        s->doing &= ~ATSP_REFERENCE;
                        /* TODO check if papers lenght mismatch */
                    }
                    return;
                } else {
                    /* if there is no references then reference */
                    if (!athmotor_islimited(s->side)) {
                        atspanel_ask(s->side, ATSP_REFERENCE);
                        return;
                    }
                }

                s->aauto.wait = 0.0;
                s->aauto.dir  = -1;
                s->aauto.trg  = 1;
                s->aauto.wfd  = 3.0;

                s->doing |= ATSP_AREADY;
                return;
            }
            
            /* door open? */
            if (athin_switchedon(s->door)) {
                s->doing &= ~ATSP_AREADY;
                return;
            }

            /* do run! */
            if (s->aauto.wait <= 0.0) {
                /* find next position */
                s->aauto.trg = getnexti_greedy(s->trgs, s->aauto.trg,
                    &s->aauto.dir);
                s->aauto.wait = s->trgs[s->aauto.trg].duration;

                /* go to it */
                athmotor_goto(s->side, s->trgs[s->aauto.trg].target,
                    ATHM_STICKY);
            } else
            if (!athmotor_targeted(s->side)) {
                /* going to it... */
                athlcd_printf(1, "A ir p/ folha %d", s->aauto.trg + 1);
            } else {
                /* waiting */
                s->aauto.wait -= dt;
                athlcd_printf(1, "[%2d] Prox %s", s->aauto.trg + 1,
                    ats_time_tos(s->aauto.wait, 1));
            }
        } else
        if (s->doing & ATSP_SMANUAL) { /* MANUAL MODE */
            /* do reference */
            if (s->doing & ATSP_REFERENCE) {
                if (!reference(s, dt)) {
                    s->doing &= ~ATSP_REFERENCE;
                    /* TODO check if papers lenght mismatch */
                }
                return;
            }
        }
    }
}


#define STOP_THRESHOLD        0.1
#define DIFF_THRESHOLD        (INTER_PAGE  0.5)
#define LIMIT_CALIB           0.25

#define INTER_PAGE            6.0
#define INTER_PRE             0.0
#define INTER_POS             0.0

void reference_init(pstate * s) {
    struct aref * r = &s->aref;
    r->state     = 0;
    r->safe_dist = -1.0;
    r->wait      = -1.0;
    r->mismatch  = 1;

    athlcd_printf(1, "{R} Referenciar");

    /* TODO TEST PORPUSES */
    //r->wait      = 2.0;
}

uint8_t reference(pstate * s, double dt) {
    struct aref * r = &s->aref;
    if (r->wait >= 0.0) r->wait -= dt;

    /* TODO TEST PORPUSES */
    //if (r->wait > 0.0) {
    //    athlcd_printf(1, "A cagar...");
    //} else {
    //    athmotor_set_limits(s->side, 0, 0);
    //    r->state     = -1;
    //    return 0;
    //}
    //return 1;

    if (r->state  == 0) { /* init sequence */
        athdecoder_reset(s->side); /* reset decoder */
        r->safe_dist = athmotor_position(s->side);
        athmotor_gos(s->side, ATHM_DOWN, ATHM_SLOW);
        /* reset motor limits */
        athmotor_unset_limits(s->side);
        r->wait = 0.5;
        r->state = 1;
        athlcd_printf(1, "{R} Inicio");
    } else
    if (r->state  == 1) { /* rolling down */
        if (fabs(athmotor_rps(s->side)) < STOP_THRESHOLD &&
                r->wait < 0.0) {
            athmotor_go(s->side, ATHM_BRAKE | ATHM_HARD);
            r->wait = 0.25;
            r->state = 2;
        }
    } else
    if (r->state  == 2) { /* wait stoped a bit and reset */
        if (r->wait < 0.0) {
            r->safe_dist = r->safe_dist -
                athmotor_position(s->side) - 3.0;
            athdecoder_reset(s->side);
            r->limit_start = athmotor_position(s->side) + LIMIT_CALIB;
            athmotor_gos(s->side, ATHM_UP, ATHM_FAST);
            r->wait = 0.5;
            r->state = 3;
            athlcd_printf(1, "{R} Fim");
        }
    } else
    if (r->state  == 3) { /* rolling up */
        if (athmotor_position(s->side) > r->safe_dist) {
            athmotor_gos(s->side, ATHM_UP, ATHM_SLOW);
        }
        if (fabs(athmotor_rps(s->side)) < STOP_THRESHOLD &&
                r->wait < 0.0) {
            athmotor_go(s->side, ATHM_BRAKE | ATHM_HARD);
            r->wait = 0.25;
            r->state = 4;
        }
    } else
    if (r->state  == 4) {  /* wait stoped a bit */
        if (r->wait < 0.0) {
            r->limit_end = athmotor_position(s->side) - LIMIT_CALIB; // TODO force get postition
            r->lenght    = r->limit_end - r->limit_start;
            r->state = 5;
            athlcd_printf(1, "{R} Esticar");

            /* go to the begining */
            athmotor_gotos(s->side, r->limit_start, ATHM_ONESHOT, ATHM_FAST);
        }
    } else
    if (r->state  == 5) { /* wait for reach the begining and exit */
        if (athmotor_targeted(s->side)) {
            /* interpolate how many pages are here */
            double est = (r->lenght - INTER_PRE - INTER_POS) / INTER_PAGE;
            uint8_t i;
            for (i = 0; i < ATSP_MAXTARGETS; i++) {
                if (i < est) {
                    r->trgs[i].inuse    = 1;
                    r->trgs[i].target   =
                        LIMIT_CALIB + INTER_PRE + INTER_PAGE * i;
                    r->trgs[i].duration = 0.0;
                } else { /* reset */
                    r->trgs[i].inuse    = 0;
                    r->trgs[i].target   = 0.0;
                    r->trgs[i].duration = 0.0;
                }
            }
            /* check if they match */
            if (fabs(settings.lenght[s->side] - r->lenght) <
                    ATSP_MISMATCH_THRESHOLD) {
                r->mismatch = 0;
            }

            /* copy values to the panel's settings struct */
            settings.limit_start[s->side] = r->limit_start;
            settings.limit_end[s->side]   = r->limit_end;
            settings.lenght[s->side]      = r->lenght;

            /* tell the moters its limits */
            athmotor_set_limits(s->side, r->limit_start, r->limit_end);

            /* exit */
            athlcd_printf(1, "");

            r->state     = 0;
            r->safe_dist = -1.0;
            r->wait      = -1.0;
            return 0;
        }
    } else {
        /* wrong state, reset it */
        r->state     = -1;
        r->safe_dist = -1.0;
        r->wait      = -1.0;
        return 0;
    }
    return 1;
}