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
ATSUI_M         uistate;
ATSUI_M         uiprevstate;

void            s_auto_init();
void            s_auto(double dt);
void            s_main_init();
void            s_main(double dt);
void            s_reference_init();
void            s_reference(double dt);
void            s_freecontrol_init();
void            s_freecontrol(double dt);
void            s_configpubs_init();
void            s_configpubs(double dt);
void            s_settings_init();
void            s_settings(double dt);
void            s_light_init();
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
void (* updatestates[ATSUI_MAXSTATES]) (double) = {
                    s_auto,
                    s_main,
                    s_reference,
                    s_freecontrol,
                    s_configpubs,
                    s_settings,
                    s_light,
                };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  SYS INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void atsui_init() {
    uiprevstate = -1;
    atsui_changestate(ATSUI_AUTO);
    //atsui_changestate(ATSUI_FREECONTROL);
    //atsui_changestate(ATSUI_LIGHT);
}


void atsui_update(double dt) {
    /* run the selected ui state */
    (*updatestates[uistate]) (dt);
}

void atsui_changestate(ATSUI_M s) {
    uiprevstate = uistate;
    uistate = s;
    (*initstates[uistate]) ();
}

void atsui_changeprevstate() {
    atsui_changestate(uiprevstate);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * AUTO * * * * * * * * * * * */
double led_mode_blink1a[] = {0.05, 1.95, 0.05, 1.95};
double led_mode_blink1b[] = {0.05, 0.1, 0.05, 0.8};

uint8_t         a_state;

void s_auto_init() {
    athlcd_clear();
    athlcd_printf(0, "      AUTO");
    athout_sequence(ATHOUT_LED1, led_mode_blink1a, 4, ATHOUT_REPEAT_YES);
    athout_off(ATHOUT_LED2);
    athout_sequence(ATHOUT_SPEAKER, led_mode_blink1b, 4, ATHOUT_REPEAT_NO_OFF);

    atspanel_ask(ATH_SIDEA, ATSP_SAUTO);
    //atspanel_ask(ATH_SIDEB, ATSP_SAUTO);

    a_state = 0;
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
int8_t m_selector;

void s_main_init() {
    athlcd_clear();
    athlcd_printf(0, "[MENU PRINCIPAL]");

    athout_sequence(ATHOUT_LED1, led_mode_blink2a, 4, ATHOUT_REPEAT_YES);
    athout_sequence(ATHOUT_LED2, led_mode_blink2b, 4, ATHOUT_REPEAT_YES);

    /* init vars */
    m_selector = 0;

    /* set panels in manual mode */
    atspanel_ask(ATH_SIDEA, ATSP_SMANUAL);
    //atspanel_ask(ATH_SIDEB, ATSP_SMANUAL);

    /* TODO gettin here also implies that the system may be consistent.
       we should reset things like references */
    // unset limits so it will reference again

    athout_dc(ATHOUT_LIGHTR, 0.7);

}

void s_main(double dt) {
    if (m_selector < 0) { /* exiting */
        if (athin_pressed(ATHIN_OK)) { /* go back to menu */
            athlcd_printf(1, "Saindo...");
        } else {
            athlcd_printf(1, "Sair?");
        }
        if (athin_clicked(ATHIN_CANCEL)) { /* go back to menu */
            m_selector = 0;
            return;
        }
        if (athin_longpressed(ATHIN_OK)) { /* go back to menu */
            atsui_changestate(ATSUI_AUTO);
            return;
        }
        return;
    }

    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        m_selector = -1;
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) m_selector++;
    if (athin_clicking(ATHIN_LEFT))  m_selector--;

    if (m_selector < 0) m_selector = 6 - 1;
    m_selector = abs(m_selector) % (6);

    switch (m_selector) {
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
                m_selector = -1;
                return;
            }
            break;
    }
}

/* * * * * * * * * * * * REFERENCE * * * * * * * * * * * */
int8_t r_selector;

void s_reference_init() {
    athlcd_clear();
    athlcd_printf(0, "[REFERENCIAR]");

    r_selector = 0;
}

void s_reference(double dt) {
    if (atspanel_isdoing(ats_wside(), ATSP_REFERENCE)) { /* referencing... */
        r_selector = 1;
        return;
    }

    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) r_selector++;
    if (athin_clicking(ATHIN_LEFT))  r_selector--;

    if (r_selector < 0) r_selector = 2 - 1;
    r_selector = abs(r_selector) % (2);

    switch (r_selector) {
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
int8_t f_mode;
uint8_t f_slow;

void s_freecontrol_init() {
    athlcd_clear();
    athlcd_printf(0, "CONTROLO LIVRE");

    f_mode = 0;
    f_slow = 0;
}

void s_freecontrol(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        //atspanel_brake(ats_wside());
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) f_mode++;
    if (athin_clicking(ATHIN_LEFT))  f_mode--;

    if (f_mode < 0) f_mode = 4 - 1;
    f_mode = abs(f_mode) % (4);

    if (athin_clicked(ATHIN_OK)) f_slow ^= 1;

    if (f_mode == 0) { /* normal mode */
        athlcd_printf(0, "[L] Normal     %c", f_slow ? '-' : ' ');
        atspanel_hobble_disable(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, f_slow);
    } else
    if (f_mode == 1) { /* hobble up */
        athlcd_printf(0, "[L] Superior   %c", f_slow ? '-' : ' ');
        atspanel_hobble_up(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, f_slow);
    } else
    if (f_mode == 2) { /* hobble down */
        athlcd_printf(0, "[L] Inferior   %c", f_slow ? '-' : ' ');
        atspanel_hobble_down(ats_wside());
        atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, f_slow);
    } else
    if (f_mode == 3) { /* free */
        atspanel_hobble_disable(ats_wside());
        athlcd_printf(0, "[L] Livre");
        atspanel_free(ats_wside());
    }


    athlcd_printf(1, "%6.2fr %5.2frps", athdecoder_position(ats_wside()),
        athdecoder_rps(ats_wside()));

}

/* * * * * * * * * * * * CONFIGPUBS * * * * * * * * * * * */
int8_t        c_state;
int8_t        c_smenu;
uint8_t       c_unc;
int8_t        c_ctrg;
uint8_t       c_save;

void s_configpubs_init() {
    athlcd_clear();
    athlcd_printf(0, "CONFIG PUBS");

    c_state = 0;
    c_smenu = -1;
    c_ctrg  = 0;
    c_save  = 0;
    c_unc   = 0;
}

void s_configpubs(double dt) {
    // TODO check for save modification on exiting
    if (c_state < 0) {
        athlcd_printf(0, "[C] Gravar?");

        if      (athin_clicked(ATHIN_UP))   c_save = 1;
        else if (athin_clicked(ATHIN_DOWN)) c_save = 0;

        if (c_save) athlcd_printf(1, "Sim");
        else        athlcd_printf(1, "Nao");

        if (athin_clicked(ATHIN_CANCEL)) {
            //atsui_changestate(ATSUI_REFERENCE);
            c_state = 1;
        }
        if (athin_clicked(ATHIN_OK)) {
            if (c_save) {
                atspanel_savetargets(ats_wside());
            } else {
            
            }
            atsui_changestate(ATSUI_REFERENCE);
        }
        return;
    }

    if (c_state == 0) {
        if (athin_clicked(ATHIN_CANCEL)) { /* go back */
            atsui_changestate(ATSUI_REFERENCE);
            return;
        }

        if (atspanel_mismatched(ats_wside())) {
            athlcd_printf(0, "[C] Nova cfg? %c", c_unc ? 'S' : 'N');
            athlcd_printf(1, "Folhas: %d vs %d",
                atspanel_counttrgs_active(atspanel_getrefstmp(ats_wside())),
                atspanel_counttrgs_active(atspanel_getrefs(ats_wside())));

            if      (athin_clicked(ATHIN_UP))   c_unc = 1;
            else if (athin_clicked(ATHIN_DOWN)) c_unc = 0;

            if (athin_clicked(ATHIN_OK)) {
                if (!c_unc) {
                    atspanel_copytrgs(atspanel_getrefs(ats_wside()),
                        atspanel_getrefstmp(ats_wside()));
                }
                c_state = 1;
            }
        } else {
            atspanel_copytrgs(atspanel_getrefs(ats_wside()),
                atspanel_getrefstmp(ats_wside()));
            c_state = 1;
        }
    } else
    if (c_state == 1) {
        atsp_target * trgs = atspanel_getrefstmp(ats_wside());

        if (c_smenu == 0) { /* position/time */
            if (athin_clicked(ATHIN_OK) || athin_clicked(ATHIN_CANCEL)) {
                c_smenu = -1;
            }
            /* if editing, then it will be used */
            trgs[c_ctrg].inuse = 1;

            if (athin_clicking(ATHIN_RIGHT)) trgs[c_ctrg].duration += 1.0;
            if (athin_clicking(ATHIN_LEFT))  trgs[c_ctrg].duration -= 1.0;

            /* no negative time, please */
            if (trgs[c_ctrg].duration < 0) trgs[c_ctrg].duration = 0.0;

            /* fast-lane to the time */
            if (trgs[c_ctrg].duration > 150.0) { /* > 2:30m */
                if (athin_clicking(ATHIN_RIGHT) && athin_longpressed(ATHIN_RIGHT))
                    trgs[c_ctrg].duration += 29.0;
                if (athin_clicking(ATHIN_LEFT) && athin_longpressed(ATHIN_LEFT))
                    trgs[c_ctrg].duration -= 29.0;
            }

            uint8_t slow = 1;
            if (athin_longpressed(ATHIN_UP) || athin_longpressed(ATHIN_DOWN)) {
                slow = 0;
            }
            atspanel_walk(ats_wside(), ATHIN_UP, ATHIN_DOWN, slow);

            athlcd_printf(0, "[C] Folha %2d?", c_ctrg + 1);
            athlcd_printf(1, "P: --.--, %s",
                ats_time_tos(trgs[c_ctrg].duration, 0));

            if (athmotor_islimited(ats_wside())) {
                trgs[c_ctrg].target = athmotor_position(ats_wside());
                athlcd_printf(1, "P: %5.2f, %s", trgs[c_ctrg].target,
                    ats_time_tos(trgs[c_ctrg].duration, 0));
            }

        } else { /* trg selector */

            if (athin_clicking(ATHIN_RIGHT)) c_ctrg++;
            if (athin_clicking(ATHIN_LEFT))  c_ctrg--;

            if (c_ctrg < 0) c_ctrg = ATSP_MAXTARGETS - 1;
            c_ctrg = abs(c_ctrg) % (ATSP_MAXTARGETS);


            if ((athin_clicking(ATHIN_RIGHT) || athin_clicking(ATHIN_LEFT)) &&
                athmotor_islimited(ats_wside())) {
                athmotor_goto(ats_wside(), trgs[c_ctrg].target, ATHM_ONESHOT);
            }

            athlcd_printf(0, "[C] Folha %2d (%c)", c_ctrg + 1,
                trgs[c_ctrg].inuse ? 'S' : 'N');
            athlcd_printf(1, "P: %5.2f, %s", trgs[c_ctrg].target,
                ats_time_tos(trgs[c_ctrg].duration, 0));

            if      (athin_clicked(ATHIN_UP))   trgs[c_ctrg].inuse = 1;
            else if (athin_clicked(ATHIN_DOWN)) trgs[c_ctrg].inuse = 0;

            if (athin_clicked(ATHIN_OK)) {
                c_smenu = 0;
                return;
            }
            if (athin_clicked(ATHIN_CANCEL)) { /* go back */
                c_state = -1;
                c_save  = 0;
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

void s_settings(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) { /* go back */
        atsui_changestate(ATSUI_MAIN);
        return;
    }
}





/* * * * * * * * * * * * LIGHT * * * * * * * * * * * */
int8_t l_led;
double l_dc[3];
double l_hsl[3];
double blink;
uint8_t l_light = 0;

void s_light_init() {
    athlcd_clear();
    athlcd_printf(0, "LIGHT CONTROL");

    l_led = 0;

    l_hsl[0] = 0.0;
    l_hsl[1] = 1.0;
    l_hsl[2] = 0.5;
    blink = 0.0;

    l_dc[0] = 0.0;
    l_dc[1] = 1.0;
    l_dc[2] = 1.0;
    
    athrgb_fadeto(ATHRGB_P1, 1.0, 1.0, 1.0, 2.0, ATHRGB_RGB);
}

void s_light(double dt) {
    if (athin_clicked(ATHIN_CANCEL)) {
        atsui_changestate(ATSUI_MAIN);
        //atspanel_brake(ats_wside());
        return;
    }

    if (athin_clicking(ATHIN_RIGHT)) l_led++;
    if (athin_clicking(ATHIN_LEFT))  l_led--;

    #define L_MMAX 3
    if (l_led < 0) l_led = L_MMAX - 1;
    l_led = abs(l_led) % (L_MMAX);

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
        l_light ^= 1;
        athrgb_fadeto(ATHRGB_P1,(double) l_light, (double) l_light, (double) l_light, 2.0, ATHRGB_RGB);
        athlcd_printf(1, "%.3f", (double) l_light);
    }

    //blink += dt;
    //l_hsl[0] += dt/5.0;
    //l_hsl[2] = (sin(blink * M_PI * 5.0) / 2.0) + 0.5;

    if (athin_clicking(ATHIN_UP)) {
        l_hsl[l_led] += L_SPEED;
        if (l_hsl[l_led] > 1.0) l_hsl[l_led] -= 1.0;
    } else
    if (athin_clicking(ATHIN_DOWN)) {
        l_hsl[l_led] -= L_SPEED;
        if (l_hsl[l_led] < 0.0) l_hsl[l_led] += 1.0;
    } else

    athlcd_printf(0, "[%c] LIGHT CONTROL",
        l_led == 0 ? 'H' : (l_led == 1 ? 'S' : 'L'));
    athlcd_printf(1, "%.2f, %.2f %.2f", l_hsl[0], l_hsl[1], l_hsl[2]);

    //l_hsl[0] = fabs(fmod(l_hsl[0], 1.0));
    //l_hsl[1] = fabs(fmod(l_hsl[1], 1.0));
    //l_hsl[2] = fabs(fmod(l_hsl[2], 1.0));
    //HSLtoRGB(l_hsl[0], l_hsl[1], l_hsl[2], l_dc);

    //athrgb_hsl(ATHRGB_P1, l_hsl[0], l_hsl[1], l_hsl[2]);
    //athrgb_rgb(ATHRGB_P1, 1, 1, 1);

    athrgb_fadeto(ATHRGB_P1,
        l_hsl[0], l_hsl[1], l_hsl[2], 0.5, ATHRGB_HSL);
    //athout_dc(ATHOUT_LIGHTR, 0.5*LEDEXP(l_dc[0], 0.01, 1));
    //athout_dc(ATHOUT_LIGHTG, LEDEXP(l_dc[1], 0.001, 1));
    //athout_dc(ATHOUT_LIGHTB, LEDEXP(l_dc[2], 2, 1));

}

