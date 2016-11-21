#include "ats.h"
/*  TODO
    - always call referencing. only reference if ->doing & reerence > 0
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* system global settings */
typedef struct syssettings {
    uint8_t     doublesided;
                /* motors settings */
    //double      limit_start[ATH_SIDES];
    //double      limit_end[ATH_SIDES];
    //double      lenght[ATH_SIDES];

    //atsp_target target[ATH_SIDES][ATSP_MAXTARGETS];
} syssettings;
syssettings settings = {0};

/* state */
typedef struct pstate {
    ATSP_ASK        doing;

    ATSP_ERR        error;

    uint8_t         side;
    uint8_t         door;
    uint8_t         paper;

    /* paper stuff */
    atsp_target     trgs[ATSP_MAXTARGETS];
    double          lenght;

    /*
        TASK SPECIFIC
    */
    /* autoing */
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
        //atsp_target trgs[ATSP_MAXTARGETS];
    } aref;

} pstate;
pstate state[ATH_SIDES] = {0};

void        init_panel(pstate * s, uint8_t side, uint8_t door, uint8_t paper);
void        update_panel(double dt, pstate * s);

void        reference_init(pstate * s);
uint8_t     reference(pstate * s, double dt);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 SYS INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void atspanel_init() {
    /* init structure and state */
    init_panel(&state[ATH_SIDEA], ATH_SIDEA, ATHIN_DOOR, ATHIN_PAPER);
    init_panel(&state[ATH_SIDEB], ATH_SIDEB, ATHIN_DOOR, ATHIN_PAPER);

    /* init semaphore */
    //ath_seminit(&locksem);

    /* initalize motors state */


    /* postitions test */
    settings.target[ATH_SIDEA][0].inuse    = 1;
    settings.target[ATH_SIDEA][0].target   = 2.0;
    settings.target[ATH_SIDEA][0].duration = 5.0;
    settings.target[ATH_SIDEA][1].inuse    = 1;
    settings.target[ATH_SIDEA][1].target   = 5.0;
    settings.target[ATH_SIDEA][1].duration = 7.5;
    settings.target[ATH_SIDEA][2].inuse    = 1;
    settings.target[ATH_SIDEA][2].target   = 8.0;
    settings.target[ATH_SIDEA][2].duration = 10.0;

    /* postitions test */
    settings.target[ATH_SIDEB][0].inuse    = 1;
    settings.target[ATH_SIDEB][0].target   = 2.0;
    settings.target[ATH_SIDEB][0].duration = 7.5;
    settings.target[ATH_SIDEB][1].inuse    = 1;
    settings.target[ATH_SIDEB][1].target   = 5.0;
    settings.target[ATH_SIDEB][1].duration = 10.0;
    settings.target[ATH_SIDEB][2].inuse    = 1;
    settings.target[ATH_SIDEB][2].target   = 8.0;
    settings.target[ATH_SIDEB][2].duration = 5.0;


}

void atspanel_update(double dt) {
    /* TODO control both panel in mutual exclusion */


    update_panel(dt, &state[ATH_SIDEA]);
    update_panel(dt, &state[ATH_SIDEB]);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void atspanel_ask(uint8_t side, ATSP_ASK ask) {
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
    if (ask & ATSP_SSAFE) { /* beeing asked to go auto mode */
        state[side].doing  = ATSP_SSAFE;

        /* break the motors */
        athmotor_go(side, ATHM_BRAKE);
        atspanel_free(side);

        /* tell UI that this is in safe mode */
    }
    if (ask & ATSP_SAUTO) { /* beeing asked to go auto mode */

        /* reset states after this big asking */
        //s->asking = 0;
        state[side].doing = ATSP_SAUTO;
        state[side].error = ATSP_ERR_DIRTY;
    }
    if (ask & ATSP_SMANUAL) { /* beeing asked to go auto mode */
        /* TODO check any inconvenience */
        /* if already in manual, ignore the ask */
        if (state[side].doing & ATSP_SMANUAL)
            return;

        /* reset env things we dunno if they changed */
        atspanel_inconsistent(side);

        athmotor_go(side, ATHM_BRAKE);

        /* reset states after this big asking */
        //s->asking = 0;
        state[side].doing = ATSP_SMANUAL;
    }
}

uint8_t atspanel_isdoing(uint8_t side, ATSP_ASK ask) {
    return (state[side].doing & ask);
}

uint8_t atspanel_mismatched(uint8_t side) {
    return state[side].aref.mismatch;
}

double atspanel_getlatestart(uint8_t side) {
    return state[side].aauto.wfd;
}

double atspanel_get_nextjump(uint8_t side) {
    return state[side].aauto.wait;
}

uint8_t atspanel_get_actualtrg(uint8_t side) {
    return state[side].aauto.trg;
}

uint8_t atspanel_is_targeted(uint8_t side) {
    return athmotor_targeted(side);
}

atsp_target * atspanel_getrefstmp(uint8_t side) {
    //return state[side].aref.trgs;
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

uint8_t atspanel_counttrgs_useful(uint8_t side) {
    uint8_t c = 0, i;
    for (i = 0; i < ATSP_MAXTARGETS; i++) {
        if (state[side].trgs[i].inuse &&
            state[side].trgs[i].duration >= 1.0) c++;
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

    //uint8_t i;
    //for (i = 0; i < ATSP_MAXTARGETS; i++) {
    //    state[side].trgs[i] = state[side].aref.trgs[i];
    //}
}

void atspanel_walk(uint8_t side, uint8_t keyup, uint8_t keydown, uint8_t slow) {
    if (athin_clicked(keyup) || athin_longclicked(keyup)) {
        athmotor_gos(ats_wside(), ATHM_UP, slow ? ATHM_SLOW : ATHM_NORMAL);
    } else
    if (athin_clicked(keydown) || athin_longclicked(keydown)) {
        athmotor_gos(ats_wside(), ATHM_DOWN, slow ? ATHM_SLOW : ATHM_NORMAL);
    }
    if (athin_released(keyup) || athin_released(keydown)) {
        athmotor_go(ats_wside(), ATHM_BRAKE);
    }
}

void atspanel_hobble_up(uint8_t side) {
    athmotor_which(side, ATHM_TOP);
}

void atspanel_hobble_down(uint8_t side) {
    athmotor_which(side, ATHM_BOTTOM);
}

void atspanel_hobble_disable(uint8_t side) {
    athmotor_which(side, ATHM_BOTH);
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

void atspanel_error_add(uint8_t side, ATSP_ERR err) {
    state[side].error = ATSP_ERR_DIRTY | err;
}

void atspanel_error_clear(uint8_t side, ATSP_ERR err) {
    state[side].error &= ATSP_ERR_DIRTY | (~err);
    if (state[side].error == ATSP_ERR_DIRTY)
        state[side].error = 0;
}

uint8_t atspanel_error_check(uint8_t side, ATSP_ERR err) {
    return (state[side].error & err);
}

uint8_t atspanel_error_erroring(uint8_t side) {
    return state[side].error;
}

void atspanel_error_clearall(uint8_t side) {
    state[side].error = 0;
}





/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void init_panel(pstate * s, uint8_t side, uint8_t door, uint8_t paper) {
    s->doing = ATSP_OFF;
    s->error = 0;

    s->side  = side;
    s->door  = door;
    s->paper = paper;

    s->trgs  = settings.target[side];
}
uint8_t getnexti_greedy(atsp_target * t, int8_t i, int8_t *dir) {
    int8_t oi = i;
    i += *dir;
    while (i >= 0 && i < ATSP_MAXTARGETS) {
        if (t[i].inuse && t[i].duration >= 1.0) { /* found */
            return i;
        }
        i += *dir;
    }
    *dir *= -1;
    i = oi;
    i += *dir;
    while (i >= 0 && i < ATSP_MAXTARGETS) {
        if (t[i].inuse && t[i].duration >= 1.0) { /* found */
            return i;
        }
        i += *dir;
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
            /* check and mark errors: DOOR */
            if (athin_switchedon(s->door)) {
                atspanel_error_add(s->side, ATSP_ERR_DOOR);
                atspanel_inconsistent(s->side);
            } else {
                atspanel_error_clear(s->side, ATSP_ERR_DOOR);
            }
            /* check and mark errors: PAPER */
            if (athin_switchedon(s->paper)) {
                atspanel_error_add(s->side, ATSP_ERR_PAPER);
                atspanel_inconsistent(s->side);
            } else {
                //atspanel_error_clear(s->side, ATSP_ERR_PAPER);
            }
            /* check and mark errors: TARGETS */
            if (atspanel_counttrgs_useful(s->side) < 1) {
                atspanel_error_add(s->side, ATSP_ERR_NOTRGS);
            } else {
                atspanel_error_clear(s->side, ATSP_ERR_NOTRGS);
            }

            /* if some error occured, mark this panel as not ready */
            if (atspanel_error_erroring(s->side)) {
                s->doing &= ~ATSP_AREADY;
            }


            if (!(s->doing & ATSP_AREADY)) {
                /* check of errors */
                if (atspanel_error_erroring(s->side)) {
                    athmotor_go(s->side, ATHM_BRAKE);
                    s->aauto.wfd = 3.0 * (s->side + 1);
                    return;
                }

                /* late start after error recovering */
                if (s->aauto.wfd > 0.0) {
                    s->aauto.wfd -= dt;
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

                s->doing |= ATSP_AREADY;
                return;
            }

            /* do run! */
            if (s->aauto.wait <= 0.0) { /* time's over, get next! */
                /* find next position */
                s->aauto.trg = getnexti_greedy(s->trgs, s->aauto.trg,
                    &(s->aauto.dir));
                s->aauto.wait = s->trgs[s->aauto.trg].duration;

                /* go to it */
                athmotor_goto(s->side, s->trgs[s->aauto.trg].target,
                    ATHM_STICKY);

            } else { /* waiting 'til the next! */
                if (!athmotor_targeted(s->side)) {
                } else {
                    /* waiting */
                    s->aauto.wait -= dt;
                }
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
#define LIMIT_CALIB           0.5

#define INTER_PAGE            6.0
#define INTER_PRE             0.0
#define INTER_POS             0.0

void reference_init(pstate * s) {
    struct aref * r = &s->aref;
    r->state     = 0;
    r->safe_dist = -1.0;
    r->wait      = -1.0;
    r->mismatch  = 1;

    //athlcd_printf(1, "{R} Referenciar");
}

uint8_t reference(pstate * s, double dt) {
    struct aref * r = &s->aref;
    if (r->wait >= 0.0) r->wait -= dt;

    if (r->state  == 0) { /* init sequence */
        athdecoder_reset(s->side); /* reset decoder */
        r->safe_dist = athmotor_position(s->side);
        atspanel_hobble_down(s->side);
        athmotor_gos(s->side, ATHM_DOWN, ATHM_SLOW);
        /* reset motor limits */
        athmotor_unset_limits(s->side);
        r->wait = 0.5;
        r->state = 1;
        //athlcd_printf(1, "{R} Inicio"); TODO
    } else
    if (r->state  == 1) { /* rolling down */
        if (fabs(athmotor_rps(s->side)) < STOP_THRESHOLD &&
                r->wait < 0.0) {
            atspanel_hobble_disable(s->side);
            athmotor_go(s->side, ATHM_BRAKE | ATHM_HARD);
            r->wait = 1.25;
            r->state = 2;
        }
    } else
    if (r->state  == 2) { /* wait stoped a bit and reset */
        if (r->wait < 0.0) {
            r->safe_dist = r->safe_dist -
                athmotor_position(s->side) - 3.0;
            athdecoder_reset(s->side);
            r->limit_start = athmotor_position(s->side) + LIMIT_CALIB;
            atspanel_hobble_up(s->side);
            athmotor_gos(s->side, ATHM_UP, ATHM_FAST);
            r->wait = 0.5;
            r->state = 3;
            //athlcd_printf(1, "{R} Fim"); TODO
        }
    } else
    if (r->state  == 3) { /* rolling up */
        if (athmotor_position(s->side) > r->safe_dist) {
            athmotor_gos(s->side, ATHM_UP, ATHM_SLOW);
        }
        if (fabs(athmotor_rps(s->side)) < STOP_THRESHOLD &&
                r->wait < 0.0) {
            atspanel_hobble_disable(s->side);
            athmotor_go(s->side, ATHM_BRAKE | ATHM_HARD);
            r->wait = 1.25;
            r->state = 4;
        }
    } else
    if (r->state  == 4) {  /* wait stoped a bit */
        if (r->wait < 0.0) {
            r->limit_end = athmotor_position(s->side) - LIMIT_CALIB; // TODO force get postition
            r->lenght    = r->limit_end - r->limit_start;
            r->state = 5;
            //athlcd_printf(1, "{R} Esticar"); TODO

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
