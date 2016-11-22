#include "ats.h"

char time_tos_tmp[10];
int8_t  workingside = ATH_SIDEA;



void atsinit() {

    /* * * * INIT MODULES * * * */
    /* motors */
    atspanel_init();
    /* user interface */
    atsui_init();

}

void atsupdate() {
    /* TODO get loop's dt */
    double dt = ath_dt();


    /* * * * UPDATE MODULES * * * */
    /* motors */
    atspanel_update(dt);
    /* user interface */
    atsui_update(dt);
}

void ats_setwside(int8_t wside) {
    workingside = wside;
}

int8_t ats_wside() {
    return workingside;
}



char * ats_time_tos(double t, uint8_t hp) {
    if (t >= 3600.0) {       /* hours */
        uint16_t h = (uint16_t) (t / 3600);
        uint16_t m = (uint16_t) ((t - h * 3600) / 60);
        snprintf(time_tos_tmp, 10, "%2u:%02uh", h, m);
    } else
    if (t >= 100.0 || !hp) { /* minutes */
        uint16_t m = (uint16_t) (t / 60);
        uint16_t s = (uint16_t) (t - m * 60);
        snprintf(time_tos_tmp, 10, "%2u:%02um", m, s);
    } else {                 /* seconds */
        uint16_t s = (uint16_t) t;
        uint16_t cs = (uint16_t) ((t - s) * 100.0);
        if (hp == 1) snprintf(time_tos_tmp, 10, "%2u.%1us", s, (cs / 10));
        else         snprintf(time_tos_tmp, 10, "%2u.%02us", s, cs);
    }   
    return time_tos_tmp;
}


/* save/restore interface */
/*
    - save functions: void * gettosave(uint16_t * size)
    - restore functions on init functins: _init(void * restored)
    - invoke partial save: dopartialsave(enum UNIT)
*/
