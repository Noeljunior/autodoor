/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                  HW PINOUT
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  TODO
        map PORTs to PWM timers
*/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:LCD
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHLCD_PIN_RS               F2
#define ATHLCD_PIN_ENABLE           F3
#define ATHLCD_PIN_DATA             F4 /* first bit of the consecutive 4 bits */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:IN
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHIN_OK_PIN                K3
#define ATHIN_CANCEL_PIN            K0
#define ATHIN_UP_PIN                K2
#define ATHIN_DOWN_PIN              K4
#define ATHIN_LEFT_PIN              K1
#define ATHIN_RIGHT_PIN             K5
#define ATHIN_DOOR_PIN              D7
#define ATHIN_PAPER_PIN             L3


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:OUT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHOUT_SPEAKER_PIN          B6
#define ATHOUT_SPEAKER_PWM          ATHP_PWM_1A
#define ATHOUT_RELAY_PIN            K6
#define ATHOUT_LCDBL_PIN            H4
#define ATHOUT_LCDBL_PWM            ATHP_PWM_4B
#define ATHOUT_LCDCONTRAST_PIN      B4
#define ATHOUT_LCDCONTRAST_PWM      ATHP_PWM_2A


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:RGB
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHRGB_LEDR_PIN             H7
#define ATHRGB_LEDR_PWM             ATHP_PWM_4A
#define ATHRGB_LEDG_PIN             H5
#define ATHRGB_LEDG_PWM             ATHP_PWM_4C
#define ATHRGB_LEDB_PIN             E3
#define ATHRGB_LEDB_PWM             ATHP_PWM_3A


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:MOTOR
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHMOTOR_AUP_BRAKE_PIN      B0  /* SIDE A-UP */
#define ATHMOTOR_AUP_DIR_PIN        B1
#define ATHMOTOR_AUP_FAULT_PIN      B2
#define ATHMOTOR_AUP_PWM_PIN        L4
#define ATHMOTOR_AUP_PWM_PWM        ATHP_PWM_5B
#define ATHMOTOR_ADOWN_BRAKE_PIN    L7  /* SIDE A-DOWN */
#define ATHMOTOR_ADOWN_DIR_PIN      G0
#define ATHMOTOR_ADOWN_FAULT_PIN    G1
#define ATHMOTOR_ADOWN_PWM_PIN      E4
#define ATHMOTOR_ADOWN_PWM_PWM      ATHP_PWM_3B

#define ATHMOTOR_BUP_BRAKE_PIN      B3  /* SIDE B-UP */
#define ATHMOTOR_BUP_DIR_PIN        L0
#define ATHMOTOR_BUP_FAULT_PIN      L1
#define ATHMOTOR_BUP_PWM_PIN        L5
#define ATHMOTOR_BUP_PWM_PWM        ATHP_PWM_5C
#define ATHMOTOR_BDOWN_BRAKE_PIN    L2  /* SIDE B-DOWN */
#define ATHMOTOR_BDOWN_DIR_PIN      G2
#define ATHMOTOR_BDOWN_FAULT_PIN    L6
#define ATHMOTOR_BDOWN_PWM_PIN      E5
#define ATHMOTOR_BDOWN_PWM_PWM      ATHP_PWM_3C


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:DECODER
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHDECODER_BYTE_PIN         A0
//#define ATHDECODER_SSO_PIN          C2  /* [SELECT1-C2 SELECT2-C3 OE-C4] */
#define ATHDECODER_OEN_PIN          C4
#define ATHDECODER_SEL1_PIN         C2
#define ATHDECODER_SEL2_PIN         C3
#define ATHDECODER_EN1_PIN          C6
#define ATHDECODER_EN2_PIN          C7
#define ATHDECODER_XY_PIN           C5
#define ATHDECODER_RESETX_PIN       C0
#define ATHDECODER_RESETY_PIN       C1

