/* C LIBS */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                      MODULES
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "atsys.h"

uint8_t         ats_wside();
char *          ats_time_tos(double t, uint8_t hp);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATS:PANEL
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define     ATSP_MISMATCH_THRESHOLD     1.0 /* more or less this revolutions */
#ifndef TD_ATSP_H_
#define TD_ATSP_H_
typedef enum    ATSP_ASK {
                    ATSP_OFF        = (1 << 0),
                    ATSP_SAUTO      = (1 << 1),
                    ATSP_SMANUAL    = (1 << 2),
                    ATSP_AREADY     = (1 << 3),
                    ATSP_FREE       = (1 << 4),
                    ATSP_REFERENCE  = (1 << 5),
                    ATSP_OTOP       = (1 << 6),
                    ATSP_OBOTTOM    = (1 << 7),
                } ATSP_ASK;
#endif
#ifndef TD_ATSP_S_
#define TD_ATSP_S_
typedef struct atsp_target { /* TODO day percentagem viewed */
    uint8_t         inuse;
    double          target;
    double          duration;
} atsp_target;
#endif

/* CONFIG */
#define         ATSP_MAXTARGETS         16

/* DECLARATIONS */
void            atspanel_init();
void            atspanel_update(double dt);

void            atspanel_ask(uint8_t side, uint8_t ask);
uint8_t         atspanel_isdoing(uint8_t side, uint8_t ask);
uint8_t         atspanel_mismatched(uint8_t side);
atsp_target *   atspanel_getrefstmp(uint8_t side);
atsp_target *   atspanel_getrefs(uint8_t side);
uint8_t         atspanel_counttrgs_active(atsp_target * ts);
uint8_t         atspanel_counttrgs_useful(atsp_target * ts);
void            atspanel_copytrgs(atsp_target * src, atsp_target * dst);
void            atspanel_savetargets(uint8_t side);

void            atspanel_walk(uint8_t side, uint8_t keyup, uint8_t keydown);
void            atspanel_hobble_up(uint8_t side, uint8_t keyup,
                    uint8_t keydown);
void            atspanel_hobble_down(uint8_t side, uint8_t keyup,
                    uint8_t keydown);
void            atspanel_free(uint8_t side);
void            atspanel_brake(uint8_t side);
uint8_t         atspanel_opened(uint8_t side);
uint8_t         atspanel_torn(uint8_t side);
void            atspanel_inconsistent(uint8_t side);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATS:UI
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef TD_ATSUI_M_H_
#define TD_ATSUI_M_H_
typedef enum    ATSUI_M {
                    ATSUI_AUTO,
                    ATSUI_MAIN,
                    ATSUI_REFERENCE,
                    ATSUI_FREECONTROL,
                    ATSUI_CONFIGPUB,
                    ATSUI_SETTINGS,
                ATSUI_MAXSTATES } ATSUI_M;
#endif
void            atsui_init();
void            atsui_update(double dt);
/* UI states */
void            atsui_changestate(ATSUI_M s);
void            atsui_changeprevstate();
