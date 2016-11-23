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
#include "ats_strings.h"


void            ats_setwside(int8_t wside);
int8_t          ats_wside();
char *          ats_time_tos(double t, uint8_t hp);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATS:PANEL
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define     ATSP_MISMATCH_THRESHOLD     1.0 /* more or less this revolutions */


#ifndef TD_ATSGS_H_
#define TD_ATSGS_H_
typedef enum    ATSP_SETTINGS {
                    ATSP_DOUBLESIDE = (1 << 0),
                    ATSP_RELAY      = (1 << 1),
                } ATSP_SETTINGS;
#endif
#ifndef TD_ATSP_H_
#define TD_ATSP_H_
typedef enum    ATSP_ASK {
                    ATSP_OFF        = (1 << 0),
                    ATSP_SAUTO      = (1 << 1),
                    ATSP_SMANUAL    = (1 << 2),
                    ATSP_SSAFE      = (1 << 3),
                    ATSP_AREADY     = (1 << 4),
                    ATSP_FREE       = (1 << 5),
                    ATSP_REFERENCE  = (1 << 6),
                    ATSP_OTOP       = (1 << 7),
                    ATSP_OBOTTOM    = (1 << 8),
                } ATSP_ASK;
#endif
#ifndef TD_ATSPE_H_
#define TD_ATSPE_H_
typedef enum    ATSP_ERR {
                    ATSP_ERR_CLEAR    = (0),      /* no error */
                    ATSP_ERR_DIRTY    = (1 << 0), /* dirty bit */
                    ATSP_ERR_PAPER    = (1 << 1), /* no paper detected on IR */
                    ATSP_ERR_DOOR     = (1 << 2), /* door opened */
                    ATSP_ERR_MOTOR    = (1 << 3), /* motor error */
                    ATSP_ERR_PLENMIS  = (1 << 4), /* a new reference mismatch the eeprom */
                    ATSP_ERR_NOTRGS   = (1 << 5), /* no targets defined */
                    ATSP_ERR_NOREF    = (1 << 6), /* no reference */
                } ATSP_ERR;
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
#define         ATSP_MAXTARGETS         5

/* DECLARATIONS */
void            atspanel_init();
void            atspanel_update(double dt);

void            atspanel_ask(uint8_t side, ATSP_ASK ask);
void            atspanel_stop(uint8_t side, ATSP_ASK ask);
uint8_t         atspanel_isdoing(uint8_t side, ATSP_ASK ask);
uint8_t         atspanel_mismatched(uint8_t side);
double          atspanel_getlatestart(uint8_t side);
uint8_t         atspanel_counttrgs_estimation(uint8_t side);
void            atspanel_use_estimation(uint8_t side);
atsp_target *   atspanel_getrefs(uint8_t side);
uint8_t         atspanel_counttrgs_active(uint8_t side);
uint8_t         atspanel_counttrgs_useful(uint8_t side);
double          atspanel_get_nextjump(uint8_t side);
uint8_t         atspanel_get_actualtrg(uint8_t side);
uint8_t         atspanel_is_targeted(uint8_t side);
//void            atspanel_copytrgs(atsp_target * src, atsp_target * dst);
//void            atspanel_savetargets(uint8_t side);

void            atspanel_walk(uint8_t side, uint8_t keyup, uint8_t keydown,
                    uint8_t slow);
void            atspanel_hobble_up(uint8_t side);
void            atspanel_hobble_down(uint8_t side);
void            atspanel_hobble_disable(uint8_t side);
void            atspanel_free(uint8_t side);
void            atspanel_brake(uint8_t side);
uint8_t         atspanel_opened(uint8_t side);
uint8_t         atspanel_torn(uint8_t side);
void            atspanel_inconsistent(uint8_t side);

void            atspanel_error_add(uint8_t side, ATSP_ERR err);
void            atspanel_error_clear(uint8_t side, ATSP_ERR err);
uint8_t         atspanel_error_check(uint8_t side, ATSP_ERR err);
uint8_t         atspanel_error_erroring(uint8_t side);
void            atspanel_error_clearall(uint8_t side);

void            atspanel_trgs_save(uint8_t side);
void            atspanel_trgs_reload(uint8_t side);
void            atspanel_globset_save();
void            atspanel_globset_reload();

void            atspanel_relay(int8_t enable, int8_t on, int8_t off);
void            atspanel_relay_get(int8_t * enable, int8_t * on, int8_t * off);

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
                    ATSUI_LIGHT,
                    ATSUI_RELAY,
                ATSUI_MAXSTATES } ATSUI_M;
#endif
void            atsui_init();
void            atsui_update(double dt);
/* UI states */
void            atsui_changestate(ATSUI_M s);
void            atsui_changeprevstate();

