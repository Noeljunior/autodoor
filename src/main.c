#include "athall/athall.h"
#include "atsys/atsys.h"

/*  TODO
    - gen tone by fixing width to 50% and varying the frequency
    - gen PWM by fixing the frequency and varying the width
*/

int main() {

    /* init HAL module */
    athinit();

    /* init SYS module */
    atsinit();

    /* update them */
    while (0x69) {
        athupdate();
        atsupdate();
    }

    return 1;
}


