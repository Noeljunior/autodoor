#include "ats.h"

/* TODO
    - if entering in a paper and walking, update the trg value
        only if motor is stoped
    - what is new config? after a manual reference
    - lost referencing do not imply lost limits ?!
    - slow speed during conf pubs
    - slow speed during freecontrol

*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define         UISTACK_SIZE    10

#define MAX_UPDATE_FPS      30.0

void            s_auto_init();
void            s_auto_finish();
void            s_auto(double dt);
void            s_main_init();
void            s_main_finish();
void            s_main(double dt);
void            s_reference_init();
void            s_reference_finish();
void            s_reference(double dt);
void            s_freecontrol_init();
void            s_freecontrol_finish();
void            s_freecontrol(double dt);
void            s_configpubs_init();
void            s_configpubs_finish();
void            s_configpubs(double dt);
void            s_settings_init();
void            s_settings_finish();
void            s_settings(double dt);
void            s_light_init();
void            s_light_finish();
void            s_light(double dt);
void            s_relay_init();
void            s_relay_finish();
void            s_relay(double dt);

void (* initstates[ATSUI_MAXSTATES]) () = {
                    s_auto_init,
                    s_main_init,
                    s_reference_init,
                    s_freecontrol_init,
                    s_configpubs_init,
                    s_settings_init,
                    s_light_init,
                    s_relay_init,
                };
void (* finishstates[ATSUI_MAXSTATES]) () = {
                    s_auto_finish,
                    s_main_finish,
                    s_reference_finish,
                    s_freecontrol_finish,
                    s_configpubs_finish,
                    s_settings_finish,
                    s_light_finish,
                    s_relay_finish,
                };
void (* updatestates[ATSUI_MAXSTATES]) (double) = {
                    s_auto,
                    s_main,
                    s_reference,
                    s_freecontrol,
                    s_configpubs,
                    s_settings,
                    s_light,
                    s_relay,
                };

typedef struct statestack {
    ATSUI_M stack[UISTACK_SIZE];
    uint8_t now;
} statestack;
statestack      sstack;

/* state variables */
typedef union _svars_u {
    /* AUTO */
    struct _sa {
        uint8_t     state;
        uint8_t     utrgs[ATH_SIDES];
    } a;
    /* MAIN */
    struct _sm {
        int8_t      selector;
    } m;
    /* REFERENCE */
    struct _sr {
        int8_t      selector;
    } r;
    /* FREECONTROl */
    struct _sf {
        int8_t      mode;
        uint8_t     slow;
    } f;
    /* CONFIGPUBS */
    struct _sc {
        int8_t      state;
        int8_t      smenu;
        uint8_t     unc;
        int8_t      ctrg;
        uint8_t     save;
    } c;
    /* SETTINGS */
    struct _ss {
    } s;
    /* LIGHT */
    struct _sl {
        int8_t      led;
        //double      dc[3];
        double      hsl[3];
        double      blink;
        uint8_t     light;
    } l;
    /* REALY */
    struct _srl {
        int8_t      selector;
        int8_t      enabled;
        int8_t      ontime;
        int8_t      offtime;
    } rl;
} statevars;
statevars       sv = {0};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  SYS INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void atsui_init() {
    uint8_t i;
    for (i = 0; i < UISTACK_SIZE; i++) {
        sstack.stack[i] = -1;
    }

    atsui_changestate(ATSUI_AUTO);
    //atsui_changestate(ATSUI_MAIN);
    //atsui_changestate(ATSUI_FREECONTROL);
    //atsui_changestate(ATSUI_LIGHT);

}


void atsui_update(double dt) {
    //ATH_MAX_FPS(MAX_UPDATE_FPS);

    static uint8_t blocked = 0;
    if (blocked) return;
    if (athwarranty_check() && !blocked) {
        blocked = 1;
        return;
    }

    /* run the selected ui state */
    (*updatestates[sstack.stack[sstack.now]]) (dt);
}

void atsui_changestate(ATSUI_M s) {
    /* add to stack */
    ATSUI_M prev = sstack.stack[sstack.now];
    sstack.now = (sstack.now + 1) % UISTACK_SIZE;
    sstack.stack[sstack.now] = s;

    if (finishstates[prev])
        (*finishstates[prev]) ();
    if (initstates[s])
        (*initstates[s]) ();
}

void atsui_stateup() {
    uint8_t will = sstack.now - 1;
    if (will < 0)
        will += UISTACK_SIZE;

    if (sstack.stack[will] < 0)
        return;

    sstack.now = will;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * AUTO * * * * * * * * * * * */
double led_mode_blink1a[] = {0.05, 1.95, 0.05, 1.95};
double led_mode_blink1b[] = {0.05, 0.1, 0.05, 0.8};

void s_auto_init() {
    athlcd_clear();
    //athlcd_printf(0, "      AUTO");

    ats_setwside(ATH_SIDEA);

    atspanel_ask(ATH_SIDEA, ATSP_SAUTO);
    atspanel_ask(ATH_SIDEB, ATSP_SAUTO);

    /* init svars */
    sv.a.state = 0;

    /* get the usefull targets */
    sv.a.utrgs[ATH_SIDEA] = atspanel_counttrgs_useful(ATH_SIDEA);
    sv.a.utrgs[ATH_SIDEB] = atspanel_counttrgs_useful(ATH_SIDEB);
}

void s_auto_finish() {
    /* TODO gettin here also implies that the system may be consistent.
       we should reset things like references */
    // unset limits so it will reference again

    /* stop panels from auto running */
    atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    atspanel_ask(ATH_SIDEB, ATSP_SMANUAL);
}

void s_auto(double dt) {
    /* TODO */
    if (athin_longpressed(ATHIN_OK) && athin_longpressed(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        return;
    }

    if (athin_pressed(ATHIN_OK) && athin_pressed(ATHIN_CANCEL)) {
        
        athlcd_printf(0, "   Getting out  ");
        athlcd_printf(1, "       ...      ");
        return;
    }

    /* global errors */
    if (!atspanel_isdoing(ATH_SIDEA, ATSP_SAUTO) ||
        !atspanel_isdoing(ATH_SIDEB, ATSP_SAUTO)) {
        athlcd_printf(0, "    UNKNOWN     ");
        athlcd_printf(1, "     ERROR      ");
    } else
    if (atspanel_error_check(ATH_SIDEA, ATSP_ERR_TOOHOT) ||
        atspanel_error_check(ATH_SIDEB, ATSP_ERR_TOOHOT)) {
        athlcd_printf(0, "      ERROR     ");
        athlcd_printf(1, "     TOO HOT    ");
    } else
    if (atspanel_error_check(ATH_SIDEA, ATSP_ERR_DOOR) ||
        atspanel_error_check(ATH_SIDEB, ATSP_ERR_DOOR)) {
        athlcd_printf(0, "      AUTO      ");
        athlcd_printf(1, "    Open door   ");
    } else
    if (atspanel_error_check(ATH_SIDEA, ATSP_ERR_PAPER) ||
        atspanel_error_check(ATH_SIDEB, ATSP_ERR_PAPER)) {
        athlcd_printf(0, "     ERROR      ");
        athlcd_printf(1, " PAPER TORN OUT ");
    } else
    if (atspanel_error_check(ATH_SIDEA, ATSP_ERR_MOTOR) ||
        atspanel_error_check(ATH_SIDEB, ATSP_ERR_MOTOR)) {
        athlcd_printf(0, "     ERROR      ");
        athlcd_printf(1, "  MOTOR ERROR   ");
    } else {
        #define MAXCU   7
        #define MAXCD   9
        char strbufu[ATH_SIDES][MAXCU];
        char strbufd[ATH_SIDES][MAXCD];
        uint8_t s;
        for (s = 0; s < ATH_SIDES; s++) {
            strbufu[s][0] = '\0';
            strbufd[s][0] = '\0';

            if (atspanel_error_check(s, ATSP_ERR_NOTRGS)) {
                snprintf(strbufu[s], MAXCU, " 0/ 0");
                snprintf(strbufd[s], MAXCD, "No ads");
                continue;
            } else
            if (atspanel_isdoing(s, ATSP_REFERENCE)) {
                snprintf(strbufu[s], MAXCU, "--/%2d", sv.a.utrgs[s]);
                snprintf(strbufd[s], MAXCD, "REF");
                continue;
            } else
            if (!atspanel_isdoing(s, ATSP_AREADY)) {
                /* late start */
                snprintf(strbufu[s], MAXCU, "%.1fs", atspanel_getlatestart(s));
                snprintf(strbufd[s], MAXCD, "Wait");
                continue;
            } else
            if (atspanel_isdoing(s, ATSP_AREADY)) {
                /* running */
                if (atspanel_is_targeted(s)) {
                    /* targeted */
                    snprintf(strbufu[s], MAXCU, "%2d/%2d",
                        atspanel_get_actualtrg(s) + 1,
                        sv.a.utrgs[s]);

                    snprintf(strbufd[s], MAXCD, "%.1fs",
                        atspanel_get_nextjump(s));
                } else {
                    /* targeting */
                    snprintf(strbufu[s], MAXCU, "%2d/%2d",
                        atspanel_get_actualtrg(s) + 1,
                        sv.a.utrgs[s]);

                    snprintf(strbufd[s], MAXCD, "going");
                }
                continue;
            } else {
                /* unknown error */
                snprintf(strbufu[s], MAXCU, "UNERR");
                continue;
            }
        }

        athlcd_printf(0, "%-6sAUTO%6s", strbufu[ATH_SIDEA], strbufu[ATH_SIDEB]);
        athlcd_printf(1, "%-8s%8s", strbufd[ATH_SIDEA], strbufd[ATH_SIDEB]);
    }

}

/* * * * * * * * * * * * MAIN * * * * * * * * * * * */
double led_mode_blink2a[] = {0.05, 0.1, 0.05, 0.8};
double led_mode_blink2b[] = {0.0,  0.2, 0.05, 0.75};

void s_main_init() {
    athlcd_clear();
    athlcd_printf(0, "[  MAIN MENU   ]");

    /* init svars */
    sv.m.selector = 0;

    /* set panels in manual mode */
    atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    atspanel_ask(ATH_SIDEB, ATSP_SMANUAL);

}

void s_main_finish() {

}

void s_main(double dt) {
    if (sv.m.selector < 0) { /* exiting */
        if (athin_pressed(ATHIN_OK)) { /* go back to menu */
            athlcd_printf(1, "Exiting...");
        } else {
            athlcd_printf(1, "Exit?");
        }
        if (athin_clicked(ATHIN_CANCEL)) { /* go back to menu */
            sv.m.selector = 0;
            return;
        }
        if (athin_longpressed(ATHIN_OK)) { /* go back to menu */
            atsui_changestate(ATSUI_AUTO);
            return;
        }
        return;
    }

    /* WARANTY ARMER */
    if (athin_longpressed(ATHIN_UP) && athin_longpressed(ATHIN_RIGHT) &&
            athin_longpressed(ATHIN_LEFT) && !athwarranty_isarmed()) {
        sv.m.selector = 5;
    }
    if (sv.m.selector == 5) {
        if (athin_clicked(ATHIN_CANCEL)) { /* go back */
            sv.m.selector = 0;
            return;
        }
        static uint8_t arm = 0;

        if (athin_clicked(ATHIN_UP))   arm = 1;
        if (athin_clicked(ATHIN_DOWN)) arm = 0;


        athlcd_printf(1, "Arm warranty? %c", arm ? 'Y' : 'N');

        if (arm && athin_pressed(ATHIN_OK)) {
            athlcd_printf(1, "WILL ARM WARANTY");
        }
        if (arm && athin_longpressed(ATHIN_OK)) {
            athwarranty_arm();
            sv.m.selector = 0;
        }
        if (!arm && athin_clicked(ATHIN_OK)) {
            sv.m.selector = 0;
        }

        return;
    }


    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        sv.m.selector = -1;
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) sv.m.selector++;
    if (athin_clicking(ATHIN_LEFT))  sv.m.selector--;

    if (sv.m.selector < 0) sv.m.selector = 5 - 1;
    sv.m.selector = abs(sv.m.selector) % (5);

    switch (sv.m.selector) {
        case 0: athlcd_printf(1, "> side? %c", ats_wside() == ATH_SIDEA ? 'A' :
            (ats_wside() == ATH_SIDEB ? 'B' : '-'));
            if (athin_clicked(ATHIN_OK)) {
                if (ats_wside() == ATH_SIDEA) {
                    ats_setwside(ATH_SIDEB);
                } else {
                    ats_setwside(ATH_SIDEA);
                }
            }
            break;
        case 1: athlcd_printf(1, "> free control");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_FREECONTROL);
                return;
            }
            break;
        case 2: athlcd_printf(1, "> ads config");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_REFERENCE);
                return;
            }
            break;
        case 3: athlcd_printf(1, "> relay config");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_RELAY);
                return;
            }
            break;
        /*case 4: athlcd_printf(1, "> SMS");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_SETTINGS);
                return;
            }
            break;*/
        /*case 4: athlcd_printf(1, "> settings");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_SETTINGS);
                return;
            }
            break;*/
        case 4: athlcd_printf(1, "> exit");
            if (athin_clicked(ATHIN_OK)) {
                sv.m.selector = -1;
                return;
            }
            break;
    }

    //athlcd_printf(1, "%6.2f|%6.2f", athdecoder_position(0), athdecoder_position(1));
}

/* * * * * * * * * * * * REFERENCE * * * * * * * * * * * */
void s_reference_init() {
    athlcd_clear();
    athlcd_printf(0, "[     ADS      ]");

    sv.r.selector = 0;
}

void s_reference_finish() {

}

void s_reference(double dt) {
    if (atspanel_isdoing(ats_wside(), ATSP_REFERENCE)) { /* referencing... */
        sv.r.selector = 0;
        if (athin_pressed(ATHIN_CANCEL)) { /* go back to menu */
            athlcd_printf(1, "ABORTING...");
        } else {
            athlcd_printf(1, "wait...");
        }
        if (athin_longpressed(ATHIN_CANCEL)) { /* go back to menu */
            atspanel_stop(ats_wside(), ATSP_REFERENCE);
            return;
        }
        return;
    }

    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) sv.r.selector++;
    if (athin_clicking(ATHIN_LEFT))  sv.r.selector--;

    if (sv.r.selector < 0) sv.r.selector = 2 - 1;
    sv.r.selector = abs(sv.r.selector) % (2);


    switch (sv.r.selector) {
        case 0: athlcd_printf(1, "> ads config");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_CONFIGPUB);
                return;
            }
            break;
        case 1:
            athlcd_printf(1, "> reference");
            if (athin_clicked(ATHIN_OK)) {
                atspanel_ask(ats_wside(), ATSP_REFERENCE);
                athlcd_printf(1, "wait...");
            }
            break;
    }
}

/* * * * * * * * * * * * FREECONTROL * * * * * * * * * * * */
void s_freecontrol_init() {
    athlcd_clear();
    //athlcd_printf(0, "CONTROLO LIVRE");

    sv.f.mode = 0;
    sv.f.slow = 0;
}

void s_freecontrol_finish() {
    /* back to normal state */
    atspanel_hobble_disable(ats_wside());
    athmotor_go(ats_wside(), ATHM_BRAKE);
}
int asd = 0;
void s_freecontrol(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        return;
    }


    if (athin_clicking(ATHIN_RIGHT)) sv.f.mode++;
    if (athin_clicking(ATHIN_LEFT))  sv.f.mode--;

    if (sv.f.mode < 0) sv.f.mode = 4 - 1;
    sv.f.mode = abs(sv.f.mode) % (4);

    if (athin_clicked(ATHIN_OK)) sv.f.slow ^= 1;


    if (sv.f.mode == 0) { /* normal mode */
        athlcd_printf(0, "[L] Normal     %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_disable(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 1) { /* hobble up */
        athlcd_printf(0, "[L] Up   %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_up(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 2) { /* hobble down */
        athlcd_printf(0, "[L] Down   %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_down(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 3) { /* free */
        atspanel_hobble_disable(ats_wside());
        athlcd_printf(0, "[L] Free");
        atspanel_free(ats_wside());
    }


    athlcd_printf(1, "%6.2fr %5.2frps", athdecoder_position(ats_wside()),
        athdecoder_rps(ats_wside()));

}

/* * * * * * * * * * * * CONFIGPUBS * * * * * * * * * * * */
void s_configpubs_init() {
    athlcd_clear();
    //athlcd_printf(0, "CONFIG PUBS");

    sv.c.state = 0;
    sv.c.smenu = -1;
    sv.c.ctrg  = 0;
    sv.c.save  = 0;
    sv.c.unc   = 0;
}

void s_configpubs_finish() {

}

void s_configpubs(double dt) {
    // TODO check for save modification on exiting
    if (sv.c.state < 0) {
        athlcd_printf(0, "[A] Save?");

        if      (athin_clicked(ATHIN_UP))   sv.c.save = 1;
        else if (athin_clicked(ATHIN_DOWN)) sv.c.save = 0;

        if (sv.c.save) athlcd_printf(1, "%16s", "Yes");
        else           athlcd_printf(1, "%16s", "No");

        if (athin_clicked(ATHIN_CANCEL)) {
            //atsui_changestate(ATSUI_REFERENCE);
            sv.c.state = 1;
        }
        if (athin_clicked(ATHIN_OK)) {
            if (sv.c.save) {
                //atspanel_savetargets(ats_wside());
                atspanel_trgs_save(ats_wside());
            } else {
                atspanel_trgs_reload(ats_wside());
            }
            atsui_changestate(ATSUI_REFERENCE);
        }
        return;
    }

    if (sv.c.state == 0) {
        if (athin_clicked(ATHIN_CANCEL)) { /* go back */
            atsui_changestate(ATSUI_REFERENCE);
            return;
        }

        if (atspanel_mismatched(ats_wside())) {
            athlcd_printf(0, "[A] New confg? %c", sv.c.unc ? 'Y' : 'N');
            athlcd_printf(1, "Ads: %d vs %d",
                atspanel_counttrgs_estimation(ats_wside()),
                atspanel_counttrgs_active(ats_wside()));

            if      (athin_clicked(ATHIN_UP))   sv.c.unc = 1;
            else if (athin_clicked(ATHIN_DOWN)) sv.c.unc = 0;

            if (athin_clicked(ATHIN_OK)) {
                if (sv.c.unc) {
                    atspanel_use_estimation(ats_wside());
                }
                sv.c.state = 1;
            }
        } else {
            sv.c.state = 1;
        }
    } else
    if (sv.c.state == 1) {
        atsp_target * trgs = atspanel_getrefs(ats_wside());

        if (sv.c.smenu == 0) { /* position/time */
            if (athin_clicked(ATHIN_OK) || athin_clicked(ATHIN_CANCEL)) {
                sv.c.smenu = -1;
            }
            /* if editing, then it will be used */
            trgs[sv.c.ctrg].inuse = 1;

            if (athin_clicking(ATHIN_RIGHT)) trgs[sv.c.ctrg].duration += 1.0;
            if (athin_clicking(ATHIN_LEFT))  trgs[sv.c.ctrg].duration -= 1.0;

            /* no negative time, please */
            if (trgs[sv.c.ctrg].duration < 0) trgs[sv.c.ctrg].duration = 0.0;

            /* fast-lane to the time */
            if (trgs[sv.c.ctrg].duration > 150.0) { /* > 2:30m */
                if (athin_clicking(ATHIN_RIGHT) && athin_longpressed(ATHIN_RIGHT))
                    trgs[sv.c.ctrg].duration += 29.0;
                if (athin_clicking(ATHIN_LEFT) && athin_longpressed(ATHIN_LEFT))
                    trgs[sv.c.ctrg].duration -= 29.0;
            }


            athlcd_printf(0, "[A] Ad %2d?", sv.c.ctrg + 1);
            athlcd_printf(1, "P: --.--, %s",
                ats_time_tos(trgs[sv.c.ctrg].duration, 0));

            if (athmotor_islimited(ats_wside())) {
                uint8_t slow = 1;
                if (athin_longpressed(ATHIN_UP) || athin_longpressed(ATHIN_DOWN)) {
                    slow = 0;
                }

                //if () TODO
                atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, slow);

                trgs[sv.c.ctrg].target = athmotor_position(ats_wside());
                athlcd_printf(1, "P: %5.2f, %s", trgs[sv.c.ctrg].target,
                    ats_time_tos(trgs[sv.c.ctrg].duration, 0));
            }

        } else { /* trg selector */

            if (athin_clicking(ATHIN_RIGHT)) sv.c.ctrg++;
            if (athin_clicking(ATHIN_LEFT))  sv.c.ctrg--;

            if (sv.c.ctrg < 0) sv.c.ctrg = ATSP_MAXTARGETS - 1;
            sv.c.ctrg = abs(sv.c.ctrg) % (ATSP_MAXTARGETS);


            if ((athin_clicking(ATHIN_RIGHT) || athin_clicking(ATHIN_LEFT)) &&
                athmotor_islimited(ats_wside())) {
                athmotor_goto(ats_wside(), trgs[sv.c.ctrg].target, ATHM_ONESHOT);
            }

            athlcd_printf(0, "[A] Ad %2d (%c)", sv.c.ctrg + 1,
                trgs[sv.c.ctrg].inuse ? 'Y' : 'N');
            athlcd_printf(1, "P: %5.2f, %s", trgs[sv.c.ctrg].target,
                ats_time_tos(trgs[sv.c.ctrg].duration, 0));

            if      (athin_clicked(ATHIN_UP))   trgs[sv.c.ctrg].inuse = 1;
            else if (athin_clicked(ATHIN_DOWN)) trgs[sv.c.ctrg].inuse = 0;

            if (athin_clicked(ATHIN_OK)) {
                sv.c.smenu = 0;
                return;
            }
            if (athin_clicked(ATHIN_CANCEL)) { /* go back */
                sv.c.state = -1;
                sv.c.save  = 0;
                return;
            }
        }

    }
    

}

/* * * * * * * * * * * * SETTINGS * * * * * * * * * * * */
void s_settings_init() {
    athlcd_clear();
    athlcd_printf(0, "[   SETTINGS   ]");
}

void s_settings_finish() {

}

void s_settings(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
    }
}

/* * * * * * * * * * * * REALY * * * * * * * * * * * */
void s_relay_init() {
    athlcd_clear();
    athlcd_printf(0, "[     RELAY    ]");

    /* read from eeprom */
    atspanel_relay_get(&sv.rl.enabled, &sv.rl.ontime, &sv.rl.offtime);
}

void s_relay_finish() {
    /* save to eeprom */
    atspanel_relay(sv.rl.enabled, sv.rl.ontime, sv.rl.offtime);
}

void s_relay(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) sv.rl.selector++;
    if (athin_clicking(ATHIN_LEFT))  sv.rl.selector--;

    if (sv.rl.selector < 0) sv.rl.selector = 3 - 1;
    sv.rl.selector = abs(sv.rl.selector) % (3);

    if (sv.rl.enabled == 0)
        sv.rl.selector = 0;

    switch (sv.rl.selector) {
        case 0:
            athlcd_printf(0, "[R] Enable");
            athlcd_printf(1, "%16s", sv.rl.enabled ? "yes" : "no");
            if (athin_clicked(ATHIN_OK)) {
                sv.rl.enabled ^= 1;
                return;
            }
            break;
        case 1:
            athlcd_printf(0, "[R] Turn on");
            if (athin_clicked(ATHIN_UP))    sv.rl.ontime++;
            if (athin_clicked(ATHIN_DOWN))  sv.rl.ontime--;

            if (sv.rl.ontime < 16) sv.rl.ontime = 16;
            if (sv.rl.ontime > 18) sv.rl.ontime = 18;

            athlcd_printf(1, "     %2d:00h", sv.rl.ontime);
            break;
        case 2:
            athlcd_printf(0, "[R] Turn off");
            if (athin_clicked(ATHIN_UP))    sv.rl.offtime++;
            if (athin_clicked(ATHIN_DOWN))  sv.rl.offtime--;

            if (sv.rl.offtime < 6) sv.rl.offtime = 6;
            if (sv.rl.offtime > 9) sv.rl.offtime = 9;

            athlcd_printf(1, "     %2d:00h", sv.rl.offtime);
            break;
    }


}


/* * * * * * * * * * * * LIGHT * * * * * * * * * * * */
void s_light_init() {
    athlcd_clear();
    athlcd_printf(0, "LIGHT CONTROL");

    sv.l.led = 0;

    sv.l.hsl[0] = 0.0;
    sv.l.hsl[1] = 1.0;
    sv.l.hsl[2] = 0.5;
    sv.l.blink = 0.0;

    //sv.l.dc[0] = 0.0;
    //sv.l.dc[1] = 1.0;
    //sv.l.dc[2] = 1.0;
    
    sv.l.light = 0;
    
    //athrgb_fadeto(ATHRGB_P1, 1.0, 1.0, 1.0, 2.0, ATHRGB_RGB);
}

void s_light_finish() {
    //athrgb_flicker_off(ATHRGB_P1);
    //athrgb_fadeto(ATHRGB_P1, 0.0, 0.0, 0.0, 2.0, ATHRGB_RGB);
}

void s_light(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        return;
    }
    return;

    if (athin_clicking(ATHIN_RIGHT)) sv.l.led++;
    if (athin_clicking(ATHIN_LEFT))  sv.l.led--;

    #define L_MMAX 3
    if (sv.l.led < 0) sv.l.led = L_MMAX - 1;
    sv.l.led = abs(sv.l.led) % (L_MMAX);

    //if (athin_clicked(ATHIN_OK)) f_slow ^= 1;

    //if (f_mode == 0) { /* normal mode */
    //    athlcd_printf(0, "[R] %f     %c", f_slow ? '-' : ' ');
    //} else
    //if (f_mode == 1) { /* hobble up */
    //    athlcd_printf(0, "[G] %f   %c", f_slow ? '-' : ' ');
    //} else
    //if (f_mode == 2) { /* hobble down */
    //    athlcd_printf(0, "[B] %f   %c", f_slow ? '-' : ' ');
    //}

    #define L_SPEED 0.02

    if (athin_clicked(ATHIN_OK)) {
        sv.l.light ^= 1;
        //athrgb_fadeto(ATHRGB_P1,(double) sv.l.light, (double) sv.l.light, (double) sv.l.light, 2.0, ATHRGB_RGB);
        athlcd_printf(1, "%.3f", (double) sv.l.light);
    }

    //sv.l.blink += dt;
    //sv.l.hsl[0] += dt/5.0;
    //sv.l.hsl[2] = (sin(sv.l.blink * M_PI * 5.0) / 2.0) + 0.5;

    if (athin_clicking(ATHIN_UP)) {
        sv.l.hsl[sv.l.led] += L_SPEED;
        if (sv.l.hsl[sv.l.led] > 1.0) sv.l.hsl[sv.l.led] -= 1.0;
    } else
    if (athin_clicking(ATHIN_DOWN)) {
        sv.l.hsl[sv.l.led] -= L_SPEED;
        if (sv.l.hsl[sv.l.led] < 0.0) sv.l.hsl[sv.l.led] += 1.0;
    } else

    athlcd_printf(0, "[%c] LIGHT CONTROL",
        sv.l.led == 0 ? 'H' : (sv.l.led == 1 ? 'S' : 'L'));
    athlcd_printf(1, "%.2f, %.2f %.2f", sv.l.hsl[0], sv.l.hsl[1], sv.l.hsl[2]);

    //l_hsl[0] = fabs(fmod(l_hsl[0], 1.0));
    //l_hsl[1] = fabs(fmod(l_hsl[1], 1.0));
    //l_hsl[2] = fabs(fmod(l_hsl[2], 1.0));
    //HSLtoRGB(l_hsl[0], l_hsl[1], l_hsl[2], sv.l.d);

    //athrgb_hsl(ATHRGB_P1, l_hsl[0], l_hsl[1], l_hsl[2]);
    //athrgb_rgb(ATHRGB_P1, 1, 1, 1);

    //athrgb_fadeto(ATHRGB_P1,
    //    sv.l.hsl[0], sv.l.hsl[1], sv.l.hsl[2], 0.5, ATHRGB_HSL);
    //athout_dc(ATHOUT_LIGHTR, 0.5*LEDEXP(sv.l.d[0], 0.01, 1));
    //athout_dc(ATHOUT_LIGHTG, LEDEXP(sv.l.d[1], 0.001, 1));
    //athout_dc(ATHOUT_LIGHTB, LEDEXP(sv.l.d[2], 2, 1));

}

