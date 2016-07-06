#include "ath.h"

/* TODO
    recatoring !!
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               PRIVATE DECLARATIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )

#define TIMER0_PRESCALAR    64
#define MS_TIMER0_OVF       (clockCyclesToMicroseconds(TIMER0_PRESCALAR * 256))
#define MILLIS_INC          (MS_TIMER0_OVF / 1000)
#define FRACT_INC ((MS_TIMER0_OVF % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                  HAL INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void athtiming_init() {
    // set timer 0 prescale factor to 64
    TCCR0B |= _BV(CS01);
    TCCR0B |= _BV(CS00);
    TIMSK0 |= _BV(TOIE0);
}


void athtiming_update(double dt) {

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PUBLIC INTERFACE
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32_t athtiming_ms() {
    unsigned long m;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        m = timer0_millis;
    }
    return m;
}

uint32_t athtiming_us() {
	unsigned long m;
	uint8_t t;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
	    m = timer0_overflow_count;
	    t = TCNT0;

	    if ((TIFR0 & _BV(TOV0)) && (t < 255))
		    m++;
	}
	return ((m << 8) + t) * (TIMER0_PRESCALAR / clockCyclesPerMicrosecond());
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                 PRIVATE FUNCTIONS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ISR(TIMER0_OVF_vect) {
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}
