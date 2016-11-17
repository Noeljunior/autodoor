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

void (* initstates[ATSUI_MAXSTATES]) () = {
                    s_auto_init,
                    s_main_init,
                    s_reference_init,
                    s_freecontrol_init,
                    s_configpubs_init,
                    s_settings_init,
                    s_light_init,
                };
void (* finishstates[ATSUI_MAXSTATES]) () = {
                    s_auto_finish,
                    s_main_finish,
                    s_reference_finish,
                    s_freecontrol_finish,
                    s_configpubs_finish,
                    s_settings_finish,
                    s_light_finish,
                };
void (* updatestates[ATSUI_MAXSTATES]) (double) = {
                    s_auto,
                    s_main,
                    s_reference,
                    s_freecontrol,
                    s_configpubs,
                    s_settings,
                    s_light,
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

    atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    //atsui_changestate(ATSUI_MAIN);
    atsui_changestate(ATSUI_FREECONTROL);
    //atsui_changestate(ATSUI_LIGHT);

}


void atsui_update(double dt) {
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
    athlcd_printf(0, "      AUTO");
    //athout_sequence(ATHOUT_LED1, led_mode_blink1a, 4, ATHOUT_REPEAT_YES);
    //athout_off(ATHOUT_LED2);
    //athout_sequence(ATHOUT_SPEAKER, led_mode_blink1b, 4, ATHOUT_REPEAT_NO_OFF);

    atspanel_ask(ATH_SIDEA, ATSP_SAUTO);
    //atspanel_ask(ATH_SIDEB, ATSP_SAUTO);

    /* init svars */
    sv.a.state = 0;
}

void s_auto_finish() {
    /* TODO gettin here also implies that the system may be consistent.
       we should reset things like references */
    // unset limits so it will reference again

    /* stop panels from auto running */
    atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    //atspanel_ask(ATH_SIDEB, ATSP_SMANUAL);
}

void s_auto(double dt) {
    /* TODO */
    if (athin_longpressed(ATHIN_OK) && athin_longpressed(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        return;
    }


    if (atspanel_isdoing(ATH_SIDEA, ATSP_AREADY)) {
        athlcd_printf(0, "    AUTO: OK");
    } else {
        athlcd_printf(0, "       AUTO");
    }

}

/* * * * * * * * * * * * MAIN * * * * * * * * * * * */
double led_mode_blink2a[] = {0.05, 0.1, 0.05, 0.8};
double led_mode_blink2b[] = {0.0,  0.2, 0.05, 0.75};

void s_main_init() {
    athlcd_clear();
    athlcd_printf(0, "[MENU PRINCIPAL]");

    //athout_sequence(ATHOUT_LED1, led_mode_blink2a, 4, ATHOUT_REPEAT_YES);
    //athout_sequence(ATHOUT_LED2, led_mode_blink2b, 4, ATHOUT_REPEAT_YES);

    /* init svars */
    sv.m.selector = 0;

    /* set panels in manual mode */
    //atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    //atspanel_ask(ATH_SIDEB, ATSP_SMANUAL);

    //athout_dc(ATHOUT_LIGHTR, 0.7);

}

void s_main_finish() {

}

void s_main(double dt) {
    if (sv.m.selector < 0) { /* exiting */
        if (athin_pressed(ATHIN_OK)) { /* go back to menu */
            athlcd_printf(1, "Saindo...");
        } else {
            athlcd_printf(1, "Sair?");
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

    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        sv.m.selector = -1;
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) sv.m.selector++;
    if (athin_clicking(ATHIN_LEFT))  sv.m.selector--;

    if (sv.m.selector < 0) sv.m.selector = 6 - 1;
    sv.m.selector = abs(sv.m.selector) % (6);

    switch (sv.m.selector) {
        case 0: athlcd_printf(1, "> controlo livre");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_FREECONTROL);
                return;
            }
            break;
        case 1: athlcd_printf(1, "> referenciar");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_REFERENCE);
                return;
            }
            break;
        case 2: athlcd_printf(1, "> luz");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_LIGHT);
                return;
            }
            break;
        case 3: athlcd_printf(1, "> SMS");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_SETTINGS);
                return;
            }
            break;
        case 4: athlcd_printf(1, "> definicoes");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_SETTINGS);
                return;
            }
            break;
        case 5: athlcd_printf(1, "> sair");
            if (athin_clicked(ATHIN_OK)) {
                sv.m.selector = -1;
                return;
            }
            break;
    }
}

/* * * * * * * * * * * * REFERENCE * * * * * * * * * * * */
void s_reference_init() {
    athlcd_clear();
    athlcd_printf(0, "[REFERENCIAR]");

    sv.r.selector = 0;
}

void s_reference_finish() {

}

void s_reference(double dt) {
    if (atspanel_isdoing(ats_wside(), ATSP_REFERENCE)) { /* referencing... */
        sv.r.selector = 1;
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
        case 0:
            athlcd_printf(1, "> iniciar");
            if (athin_clicked(ATHIN_OK)) {
                atspanel_ask(ats_wside(), ATSP_REFERENCE);
            }
            break;
        case 1: athlcd_printf(1, "> config. pubs.");
            if (athin_clicked(ATHIN_OK)) {
                atsui_changestate(ATSUI_CONFIGPUB);
                return;
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
        //athlcd_printf(0, "[L] Normal     %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_disable(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 1) { /* hobble up */
        athlcd_printf(0, "[L] Superior   %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_up(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 2) { /* hobble down */
        athlcd_printf(0, "[L] Inferior   %c", sv.f.slow ? '-' : ' ');
        atspanel_hobble_down(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, sv.f.slow);
    } else
    if (sv.f.mode == 3) { /* free */
        atspanel_hobble_disable(ats_wside());
        athlcd_printf(0, "[L] Livre");
        atspanel_free(ats_wside());
    }


    //athlcd_printf(1, "%6.2fr %5.2frps", athdecoder_position(ats_wside()),
    //    athdecoder_rps(ats_wside()));
    //athlcd_printf(0, "%ld:%ld", athdecoder_read(0), athdecoder_read(1));
    //athlcd_printf(1, "y %ld", athdecoder_read(1));

}

/* * * * * * * * * * * * CONFIGPUBS * * * * * * * * * * * */
void s_configpubs_init() {
    athlcd_clear();
    athlcd_printf(0, "CONFIG PUBS");

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
        athlcd_printf(0, "[C] Gravar?");

        if      (athin_clicked(ATHIN_UP))   sv.c.save = 1;
        else if (athin_clicked(ATHIN_DOWN)) sv.c.save = 0;

        if (sv.c.save) athlcd_printf(1, "Sim");
        else           athlcd_printf(1, "Nao");

        if (athin_clicked(ATHIN_CANCEL)) {
            //atsui_changestate(ATSUI_REFERENCE);
            sv.c.state = 1;
        }
        if (athin_clicked(ATHIN_OK)) {
            if (sv.c.save) {
                atspanel_savetargets(ats_wside());
            } else {
            
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
            athlcd_printf(0, "[C] Nova cfg? %c", sv.c.unc ? 'S' : 'N');
            athlcd_printf(1, "Folhas: %d vs %d",
                atspanel_counttrgs_active(atspanel_getrefstmp(ats_wside())),
                atspanel_counttrgs_active(atspanel_getrefs(ats_wside())));

            if      (athin_clicked(ATHIN_UP))   sv.c.unc = 1;
            else if (athin_clicked(ATHIN_DOWN)) sv.c.unc = 0;

            if (athin_clicked(ATHIN_OK)) {
                if (!sv.c.unc) {
                    atspanel_copytrgs(atspanel_getrefs(ats_wside()),
                        atspanel_getrefstmp(ats_wside()));
                }
                sv.c.state = 1;
            }
        } else {
            atspanel_copytrgs(atspanel_getrefs(ats_wside()),
                atspanel_getrefstmp(ats_wside()));
            sv.c.state = 1;
        }
    } else
    if (sv.c.state == 1) {
        atsp_target * trgs = atspanel_getrefstmp(ats_wside());

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

            uint8_t slow = 1;
            if (athin_longpressed(ATHIN_UP) || athin_longpressed(ATHIN_DOWN)) {
                slow = 0;
            }
            atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, slow);

            athlcd_printf(0, "[C] Folha %2d?", sv.c.ctrg + 1);
            athlcd_printf(1, "P: --.--, %s",
                ats_time_tos(trgs[sv.c.ctrg].duration, 0));

            if (athmotor_islimited(ats_wside())) {
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

            athlcd_printf(0, "[C] Folha %2d (%c)", sv.c.ctrg + 1,
                trgs[sv.c.ctrg].inuse ? 'S' : 'N');
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
    athlcd_printf(0, "DEFINICOES");
}

void s_settings_finish() {

}

void s_settings(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
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

