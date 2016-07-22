/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *                                  HW PINOUT
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:LCD
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHLCD_PIN_RS           A0
#define ATHLCD_PIN_ENABLE       A1
#define ATHLCD_PIN_DATA         A2  /* first bit of the consecutive 4 bits */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:IN
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHIN_OK_PIN            K0
#define ATHIN_CANCEL_PIN        K5
#define ATHIN_UP_PIN            K1
#define ATHIN_DOWN_PIN          K2
#define ATHIN_LEFT_PIN          K3
#define ATHIN_RIGHT_PIN         K4
#define ATHIN_DOORA_PIN         F6
#define ATHIN_PAPERA_PIN        F7


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:OUT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHOUT_LED1_PIN         K6
#define ATHOUT_LED2_PIN         K7
#define ATHOUT_SPEAKER_PIN      F0


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:RGB
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHRGB_LEDR_PIN         B7
#define ATHRGB_LEDR_PWM         ATHP_PWM_1C
#define ATHRGB_LEDG_PIN         B6
#define ATHRGB_LEDG_PWM         ATHP_PWM_1B
#define ATHRGB_LEDB_PIN         B5
#define ATHRGB_LEDB_PWM         ATHP_PWM_1A


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:MOTOR
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define ATHMOTOR_AUP_SPEED_PIN      L5  /* DRIVER A */
#define ATHMOTOR_AUP_BRAKE_PIN      L3
#define ATHMOTOR_AUP_DIR_PIN        L7
#define ATHMOTOR_AUP_FAULT_PIN      L1
#define ATHMOTOR_AUP_PWM_PIN        L5
#define ATHMOTOR_AUP_PWM_PWM        ATHP_PWM_5C
#define ATHMOTOR_ADOWN_SPEED_PIN    L4
#define ATHMOTOR_ADOWN_BRAKE_PIN    L2
#define ATHMOTOR_ADOWN_DIR_PIN      L6
#define ATHMOTOR_ADOWN_FAULT_PIN    L0
#define ATHMOTOR_ADOWN_PWM_PIN      L4
#define ATHMOTOR_ADOWN_PWM_PWM      ATHP_PWM_5B

#define ATHMOTOR_BUP_SPEED_PIN      L5  /* DRIVER B */
#define ATHMOTOR_BUP_BRAKE_PIN      L3
#define ATHMOTOR_BUP_DIR_PIN        L7
#define ATHMOTOR_BUP_FAULT_PIN      L1
#define ATHMOTOR_BUP_PWM_PIN        5C
#define ATHMOTOR_BDOWN_SPEED_PIN    L4
#define ATHMOTOR_BDOWN_BRAKE_PIN    L2
#define ATHMOTOR_BDOWN_DIR_PIN      L6
#define ATHMOTOR_BDOWN_FAULT_PIN    L0
#define ATHMOTOR_BDOWN_PWM_PIN      5B


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                      ATH:DECODER
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
