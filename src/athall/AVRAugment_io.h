// Generated @ Sat 11-07-2015 03:17AM 48sec
/*
    TITLE:   AVR Augmentation for io.h
    Purpose: Increase portability, Readability, Save time
    AUTHOR:  Brian Khuu
    Version: V2.0
        Objective of V2:: 
            To basically allow for flexible usage of registers beyond whatever is defined in this.
                e.g.
                    Instead of:             SET_PORT( B1, 0xFF) --> Sets bit 1 @ PORTB to 1	
                    Want something like:    SET_REG(PORT, B1, OxFF) 
                    This method is more intuitive as well. But works only if the naming convention by Atmel remains consistent in this format::
                      Register Location: < REGISTER TYPE NAME (e.g. DDR ) > < REGISTER LETTER > ( e.g. DDRB )
                      Pin Naming:        < REGISTER LETTER > < Pin Number >

                    Thus what I would need to generate in masses is like this::
                      #define B1_BITMASK PB1
                      #define B1_NUMBER 1
                      #define B1_LETTER B
    Assumption:
        That you are using io.h (For avr MCU) and
        that it splits IOs into banks of Port letters each with an word sized of pin numbers
            e.g. ( PA5 -> Port A pin 5 . Is a macro within io.h )
        Port letters Supported: A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
        Pin numbers Supported: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16

        Also assumes that Pxn is defined, and that registers are named like DDRx or PINx or PORTx etc... (x=letter, n=number)

    cite: initial inspiration from cinderblock "[CODE] [C] Simplifying PORT and DDR #defines for Portability
           http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=73829
    cite: "GET" and "SET" macros from David Jahshan's game_console Template

    Example blink code for the arduino UNO in C located in bottom of this header file
*/

// These accept letter + number reference only.
//      e.g. for PA5 -> SET_PORT(A5,0xFF)
// ( R_ was a cinderblock suggestion, to allow for nested concat operations) 

// SINGLE BIT BITFIELD MASK ( Usally defined in atmel studio toolchain )
// #define _BV(bit) (1 << (bit))

// Enforce input to be either 0xFF or 0x00 (Since if people use arduino constants of HIGH --> 0x1 . Then it wont be 0xFF)
#define SET_REG_VAL_CHK(VALUE) ( (VALUE>0) ? (0xFF) : (0x00) )

//SET REGISTER
    // (1). Allows external #define to de-tokenize
#define SET_REG( REG_NAME , PIN , VALUE ) R1_SET_REG( REG_NAME , PIN , VALUE )      
    // Seperate letters and numbers. Via first the `##` concat operation, and then detokanizing the PIN_LETTER and PIN_NUMBER (e.g. B1_LETTER --> B )
#define R1_SET_REG( REG_NAME , PIN , VALUE ) R2_SET_REG( REG_NAME , PIN, PIN##_LETTER, PIN##_NUMBER, VALUE )     
#define R2_SET_REG( REG_NAME , PIN, LETTER, NUMBER, VALUE  ) R3_SET_REG( REG_NAME , PIN, LETTER, NUMBER, VALUE )
    // Now we can use the SET() macro (written by David Jahshan )
#define R3_SET_REG( REG_NAME , PIN, LETTER, NUMBER, VALUE ) SET(    REG_NAME##LETTER	,	PIN##_BITMASK	,	SET_REG_VAL_CHK(VALUE)	)

//GET REGISTER ( Similar method as SET REGISTER)
#define GET_REG( REG_NAME , PIN ) R1_GET_REG( REG_NAME , PIN )
#define R1_GET_REG( REG_NAME , PIN ) R2_GET_REG( REG_NAME , PIN, PIN##LETTER, PIN##NUMBER )
#define R2_GET_REG( REG_NAME , PIN, LETTER, NUMBER ) R3_GET_REG( REG_NAME , PIN, LETTER, NUMBER ) 
#define R3_GET_REG( REG_NAME , PIN, LETTER, NUMBER ) GET(    REG_NAME##LETTER	,	PIN##_BITMASK	)

//// From David Jahshan's SET and GET macro
// This will not accept letter+number reference, must give PORT and MASK reference
// e.g. SET_REGISTER( PORTA, _BV(PA6), 0xFF);
#define SET(REGISTER,MASK,VALUE)	REGISTER = ((MASK & VALUE) | (REGISTER & ~MASK))
#define GET(REGISTER,MASK)			(REGISTER & MASK)

// Originally from Steve Karg in http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
// This is included as reference. And also it's handy occationally.
//Mask
#define SETMASK(VARIABLE,MASK)		VARIABLE = VARIABLE | MASK
#define CLEARMASK(VARIABLE,MASK)	VARIABLE = VARIABLE & ~MASK
#define TOGGLEMASK(VARIABLE,MASK)	VARIABLE = VARIABLE ^ MASK
#define CHECKMASK(VARIABLE,MASK)	VARIABLE & MASK
//Single bit
#define SETBIT(VARIABLE,b)			((VARIABLE) |= (1<<(b)))
#define CLEARBIT(VARIABLE,b)		((VARIABLE) &= ~(1<<(b)))
#define FLIPBIT(VARIABLE,b)			((VARIABLE ^= (1<<(b)))
#define CHECKBIT(VARIABLE,b)		((VARIABLE) & (1<<(b)))


// ~~~~~~~~~~ Pin Letter and Number references ~~~~~~~~~~

// In _BV(PA1) the PA1 represents the bit position of the port in relation to the DDR,PORT,PIN
//(e.g. DDRA,PORTA,PINA) registers, etc...
// Where did PA1 etc... come from? It came from the AVR toolchain.
// It is first either directly defined as Pxn or PORTxn (e.g. in iom328p.h ), but they are essentally equivalent values
//  as seen in portpins.h which fills in either Pxn or PORTxn with the missing value from the other one.
// TL;DR: PA1 <--> PORTA1 . Both ultimately sourced from hardware header file like iom328p.h (vr8-gnu-toolchainvr\includevr)


//## Pin Letter:A

    //Pxn: A0
    #define A0_BITMASK   _BV(PA0)
    #define A0_LETTER    A
    #define A0_NUMBER    0

    //Pxn: A1
    #define A1_BITMASK   _BV(PA1)
    #define A1_LETTER    A
    #define A1_NUMBER    1

    //Pxn: A2
    #define A2_BITMASK   _BV(PA2)
    #define A2_LETTER    A
    #define A2_NUMBER    2

    //Pxn: A3
    #define A3_BITMASK   _BV(PA3)
    #define A3_LETTER    A
    #define A3_NUMBER    3

    //Pxn: A4
    #define A4_BITMASK   _BV(PA4)
    #define A4_LETTER    A
    #define A4_NUMBER    4

    //Pxn: A5
    #define A5_BITMASK   _BV(PA5)
    #define A5_LETTER    A
    #define A5_NUMBER    5

    //Pxn: A6
    #define A6_BITMASK   _BV(PA6)
    #define A6_LETTER    A
    #define A6_NUMBER    6

    //Pxn: A7
    #define A7_BITMASK   _BV(PA7)
    #define A7_LETTER    A
    #define A7_NUMBER    7

    //Pxn: A8
    #define A8_BITMASK   _BV(PA8)
    #define A8_LETTER    A
    #define A8_NUMBER    8

    //Pxn: A9
    #define A9_BITMASK   _BV(PA9)
    #define A9_LETTER    A
    #define A9_NUMBER    9

    //Pxn: A10
    #define A10_BITMASK   _BV(PA10)
    #define A10_LETTER    A
    #define A10_NUMBER    10

    //Pxn: A12
    #define A12_BITMASK   _BV(PA12)
    #define A12_LETTER    A
    #define A12_NUMBER    12

    //Pxn: A13
    #define A13_BITMASK   _BV(PA13)
    #define A13_LETTER    A
    #define A13_NUMBER    13

    //Pxn: A14
    #define A14_BITMASK   _BV(PA14)
    #define A14_LETTER    A
    #define A14_NUMBER    14

    //Pxn: A15
    #define A15_BITMASK   _BV(PA15)
    #define A15_LETTER    A
    #define A15_NUMBER    15

    //Pxn: A16
    #define A16_BITMASK   _BV(PA16)
    #define A16_LETTER    A
    #define A16_NUMBER    16


//## Pin Letter:B

    //Pxn: B0
    #define B0_BITMASK   _BV(PB0)
    #define B0_LETTER    B
    #define B0_NUMBER    0

    //Pxn: B1
    #define B1_BITMASK   _BV(PB1)
    #define B1_LETTER    B
    #define B1_NUMBER    1

    //Pxn: B2
    #define B2_BITMASK   _BV(PB2)
    #define B2_LETTER    B
    #define B2_NUMBER    2

    //Pxn: B3
    #define B3_BITMASK   _BV(PB3)
    #define B3_LETTER    B
    #define B3_NUMBER    3

    //Pxn: B4
    #define B4_BITMASK   _BV(PB4)
    #define B4_LETTER    B
    #define B4_NUMBER    4

    //Pxn: B5
    #define B5_BITMASK   _BV(PB5)
    #define B5_LETTER    B
    #define B5_NUMBER    5

    //Pxn: B6
    #define B6_BITMASK   _BV(PB6)
    #define B6_LETTER    B
    #define B6_NUMBER    6

    //Pxn: B7
    #define B7_BITMASK   _BV(PB7)
    #define B7_LETTER    B
    #define B7_NUMBER    7

    //Pxn: B8
    #define B8_BITMASK   _BV(PB8)
    #define B8_LETTER    B
    #define B8_NUMBER    8

    //Pxn: B9
    #define B9_BITMASK   _BV(PB9)
    #define B9_LETTER    B
    #define B9_NUMBER    9

    //Pxn: B10
    #define B10_BITMASK   _BV(PB10)
    #define B10_LETTER    B
    #define B10_NUMBER    10

    //Pxn: B12
    #define B12_BITMASK   _BV(PB12)
    #define B12_LETTER    B
    #define B12_NUMBER    12

    //Pxn: B13
    #define B13_BITMASK   _BV(PB13)
    #define B13_LETTER    B
    #define B13_NUMBER    13

    //Pxn: B14
    #define B14_BITMASK   _BV(PB14)
    #define B14_LETTER    B
    #define B14_NUMBER    14

    //Pxn: B15
    #define B15_BITMASK   _BV(PB15)
    #define B15_LETTER    B
    #define B15_NUMBER    15

    //Pxn: B16
    #define B16_BITMASK   _BV(PB16)
    #define B16_LETTER    B
    #define B16_NUMBER    16


//## Pin Letter:C

    //Pxn: C0
    #define C0_BITMASK   _BV(PC0)
    #define C0_LETTER    C
    #define C0_NUMBER    0

    //Pxn: C1
    #define C1_BITMASK   _BV(PC1)
    #define C1_LETTER    C
    #define C1_NUMBER    1

    //Pxn: C2
    #define C2_BITMASK   _BV(PC2)
    #define C2_LETTER    C
    #define C2_NUMBER    2

    //Pxn: C3
    #define C3_BITMASK   _BV(PC3)
    #define C3_LETTER    C
    #define C3_NUMBER    3

    //Pxn: C4
    #define C4_BITMASK   _BV(PC4)
    #define C4_LETTER    C
    #define C4_NUMBER    4

    //Pxn: C5
    #define C5_BITMASK   _BV(PC5)
    #define C5_LETTER    C
    #define C5_NUMBER    5

    //Pxn: C6
    #define C6_BITMASK   _BV(PC6)
    #define C6_LETTER    C
    #define C6_NUMBER    6

    //Pxn: C7
    #define C7_BITMASK   _BV(PC7)
    #define C7_LETTER    C
    #define C7_NUMBER    7

    //Pxn: C8
    #define C8_BITMASK   _BV(PC8)
    #define C8_LETTER    C
    #define C8_NUMBER    8

    //Pxn: C9
    #define C9_BITMASK   _BV(PC9)
    #define C9_LETTER    C
    #define C9_NUMBER    9

    //Pxn: C10
    #define C10_BITMASK   _BV(PC10)
    #define C10_LETTER    C
    #define C10_NUMBER    10

    //Pxn: C12
    #define C12_BITMASK   _BV(PC12)
    #define C12_LETTER    C
    #define C12_NUMBER    12

    //Pxn: C13
    #define C13_BITMASK   _BV(PC13)
    #define C13_LETTER    C
    #define C13_NUMBER    13

    //Pxn: C14
    #define C14_BITMASK   _BV(PC14)
    #define C14_LETTER    C
    #define C14_NUMBER    14

    //Pxn: C15
    #define C15_BITMASK   _BV(PC15)
    #define C15_LETTER    C
    #define C15_NUMBER    15

    //Pxn: C16
    #define C16_BITMASK   _BV(PC16)
    #define C16_LETTER    C
    #define C16_NUMBER    16


//## Pin Letter:D

    //Pxn: D0
    #define D0_BITMASK   _BV(PD0)
    #define D0_LETTER    D
    #define D0_NUMBER    0

    //Pxn: D1
    #define D1_BITMASK   _BV(PD1)
    #define D1_LETTER    D
    #define D1_NUMBER    1

    //Pxn: D2
    #define D2_BITMASK   _BV(PD2)
    #define D2_LETTER    D
    #define D2_NUMBER    2

    //Pxn: D3
    #define D3_BITMASK   _BV(PD3)
    #define D3_LETTER    D
    #define D3_NUMBER    3

    //Pxn: D4
    #define D4_BITMASK   _BV(PD4)
    #define D4_LETTER    D
    #define D4_NUMBER    4

    //Pxn: D5
    #define D5_BITMASK   _BV(PD5)
    #define D5_LETTER    D
    #define D5_NUMBER    5

    //Pxn: D6
    #define D6_BITMASK   _BV(PD6)
    #define D6_LETTER    D
    #define D6_NUMBER    6

    //Pxn: D7
    #define D7_BITMASK   _BV(PD7)
    #define D7_LETTER    D
    #define D7_NUMBER    7

    //Pxn: D8
    #define D8_BITMASK   _BV(PD8)
    #define D8_LETTER    D
    #define D8_NUMBER    8

    //Pxn: D9
    #define D9_BITMASK   _BV(PD9)
    #define D9_LETTER    D
    #define D9_NUMBER    9

    //Pxn: D10
    #define D10_BITMASK   _BV(PD10)
    #define D10_LETTER    D
    #define D10_NUMBER    10

    //Pxn: D12
    #define D12_BITMASK   _BV(PD12)
    #define D12_LETTER    D
    #define D12_NUMBER    12

    //Pxn: D13
    #define D13_BITMASK   _BV(PD13)
    #define D13_LETTER    D
    #define D13_NUMBER    13

    //Pxn: D14
    #define D14_BITMASK   _BV(PD14)
    #define D14_LETTER    D
    #define D14_NUMBER    14

    //Pxn: D15
    #define D15_BITMASK   _BV(PD15)
    #define D15_LETTER    D
    #define D15_NUMBER    15

    //Pxn: D16
    #define D16_BITMASK   _BV(PD16)
    #define D16_LETTER    D
    #define D16_NUMBER    16


//## Pin Letter:E

    //Pxn: E0
    #define E0_BITMASK   _BV(PE0)
    #define E0_LETTER    E
    #define E0_NUMBER    0

    //Pxn: E1
    #define E1_BITMASK   _BV(PE1)
    #define E1_LETTER    E
    #define E1_NUMBER    1

    //Pxn: E2
    #define E2_BITMASK   _BV(PE2)
    #define E2_LETTER    E
    #define E2_NUMBER    2

    //Pxn: E3
    #define E3_BITMASK   _BV(PE3)
    #define E3_LETTER    E
    #define E3_NUMBER    3

    //Pxn: E4
    #define E4_BITMASK   _BV(PE4)
    #define E4_LETTER    E
    #define E4_NUMBER    4

    //Pxn: E5
    #define E5_BITMASK   _BV(PE5)
    #define E5_LETTER    E
    #define E5_NUMBER    5

    //Pxn: E6
    #define E6_BITMASK   _BV(PE6)
    #define E6_LETTER    E
    #define E6_NUMBER    6

    //Pxn: E7
    #define E7_BITMASK   _BV(PE7)
    #define E7_LETTER    E
    #define E7_NUMBER    7

    //Pxn: E8
    #define E8_BITMASK   _BV(PE8)
    #define E8_LETTER    E
    #define E8_NUMBER    8

    //Pxn: E9
    #define E9_BITMASK   _BV(PE9)
    #define E9_LETTER    E
    #define E9_NUMBER    9

    //Pxn: E10
    #define E10_BITMASK   _BV(PE10)
    #define E10_LETTER    E
    #define E10_NUMBER    10

    //Pxn: E12
    #define E12_BITMASK   _BV(PE12)
    #define E12_LETTER    E
    #define E12_NUMBER    12

    //Pxn: E13
    #define E13_BITMASK   _BV(PE13)
    #define E13_LETTER    E
    #define E13_NUMBER    13

    //Pxn: E14
    #define E14_BITMASK   _BV(PE14)
    #define E14_LETTER    E
    #define E14_NUMBER    14

    //Pxn: E15
    #define E15_BITMASK   _BV(PE15)
    #define E15_LETTER    E
    #define E15_NUMBER    15

    //Pxn: E16
    #define E16_BITMASK   _BV(PE16)
    #define E16_LETTER    E
    #define E16_NUMBER    16


//## Pin Letter:F

    //Pxn: F0
    #define F0_BITMASK   _BV(PF0)
    #define F0_LETTER    F
    #define F0_NUMBER    0

    //Pxn: F1
    #define F1_BITMASK   _BV(PF1)
    #define F1_LETTER    F
    #define F1_NUMBER    1

    //Pxn: F2
    #define F2_BITMASK   _BV(PF2)
    #define F2_LETTER    F
    #define F2_NUMBER    2

    //Pxn: F3
    #define F3_BITMASK   _BV(PF3)
    #define F3_LETTER    F
    #define F3_NUMBER    3

    //Pxn: F4
    #define F4_BITMASK   _BV(PF4)
    #define F4_LETTER    F
    #define F4_NUMBER    4

    //Pxn: F5
    #define F5_BITMASK   _BV(PF5)
    #define F5_LETTER    F
    #define F5_NUMBER    5

    //Pxn: F6
    #define F6_BITMASK   _BV(PF6)
    #define F6_LETTER    F
    #define F6_NUMBER    6

    //Pxn: F7
    #define F7_BITMASK   _BV(PF7)
    #define F7_LETTER    F
    #define F7_NUMBER    7

    //Pxn: F8
    #define F8_BITMASK   _BV(PF8)
    #define F8_LETTER    F
    #define F8_NUMBER    8

    //Pxn: F9
    #define F9_BITMASK   _BV(PF9)
    #define F9_LETTER    F
    #define F9_NUMBER    9

    //Pxn: F10
    #define F10_BITMASK   _BV(PF10)
    #define F10_LETTER    F
    #define F10_NUMBER    10

    //Pxn: F12
    #define F12_BITMASK   _BV(PF12)
    #define F12_LETTER    F
    #define F12_NUMBER    12

    //Pxn: F13
    #define F13_BITMASK   _BV(PF13)
    #define F13_LETTER    F
    #define F13_NUMBER    13

    //Pxn: F14
    #define F14_BITMASK   _BV(PF14)
    #define F14_LETTER    F
    #define F14_NUMBER    14

    //Pxn: F15
    #define F15_BITMASK   _BV(PF15)
    #define F15_LETTER    F
    #define F15_NUMBER    15

    //Pxn: F16
    #define F16_BITMASK   _BV(PF16)
    #define F16_LETTER    F
    #define F16_NUMBER    16


//## Pin Letter:G

    //Pxn: G0
    #define G0_BITMASK   _BV(PG0)
    #define G0_LETTER    G
    #define G0_NUMBER    0

    //Pxn: G1
    #define G1_BITMASK   _BV(PG1)
    #define G1_LETTER    G
    #define G1_NUMBER    1

    //Pxn: G2
    #define G2_BITMASK   _BV(PG2)
    #define G2_LETTER    G
    #define G2_NUMBER    2

    //Pxn: G3
    #define G3_BITMASK   _BV(PG3)
    #define G3_LETTER    G
    #define G3_NUMBER    3

    //Pxn: G4
    #define G4_BITMASK   _BV(PG4)
    #define G4_LETTER    G
    #define G4_NUMBER    4

    //Pxn: G5
    #define G5_BITMASK   _BV(PG5)
    #define G5_LETTER    G
    #define G5_NUMBER    5

    //Pxn: G6
    #define G6_BITMASK   _BV(PG6)
    #define G6_LETTER    G
    #define G6_NUMBER    6

    //Pxn: G7
    #define G7_BITMASK   _BV(PG7)
    #define G7_LETTER    G
    #define G7_NUMBER    7

    //Pxn: G8
    #define G8_BITMASK   _BV(PG8)
    #define G8_LETTER    G
    #define G8_NUMBER    8

    //Pxn: G9
    #define G9_BITMASK   _BV(PG9)
    #define G9_LETTER    G
    #define G9_NUMBER    9

    //Pxn: G10
    #define G10_BITMASK   _BV(PG10)
    #define G10_LETTER    G
    #define G10_NUMBER    10

    //Pxn: G12
    #define G12_BITMASK   _BV(PG12)
    #define G12_LETTER    G
    #define G12_NUMBER    12

    //Pxn: G13
    #define G13_BITMASK   _BV(PG13)
    #define G13_LETTER    G
    #define G13_NUMBER    13

    //Pxn: G14
    #define G14_BITMASK   _BV(PG14)
    #define G14_LETTER    G
    #define G14_NUMBER    14

    //Pxn: G15
    #define G15_BITMASK   _BV(PG15)
    #define G15_LETTER    G
    #define G15_NUMBER    15

    //Pxn: G16
    #define G16_BITMASK   _BV(PG16)
    #define G16_LETTER    G
    #define G16_NUMBER    16


//## Pin Letter:H

    //Pxn: H0
    #define H0_BITMASK   _BV(PH0)
    #define H0_LETTER    H
    #define H0_NUMBER    0

    //Pxn: H1
    #define H1_BITMASK   _BV(PH1)
    #define H1_LETTER    H
    #define H1_NUMBER    1

    //Pxn: H2
    #define H2_BITMASK   _BV(PH2)
    #define H2_LETTER    H
    #define H2_NUMBER    2

    //Pxn: H3
    #define H3_BITMASK   _BV(PH3)
    #define H3_LETTER    H
    #define H3_NUMBER    3

    //Pxn: H4
    #define H4_BITMASK   _BV(PH4)
    #define H4_LETTER    H
    #define H4_NUMBER    4

    //Pxn: H5
    #define H5_BITMASK   _BV(PH5)
    #define H5_LETTER    H
    #define H5_NUMBER    5

    //Pxn: H6
    #define H6_BITMASK   _BV(PH6)
    #define H6_LETTER    H
    #define H6_NUMBER    6

    //Pxn: H7
    #define H7_BITMASK   _BV(PH7)
    #define H7_LETTER    H
    #define H7_NUMBER    7

    //Pxn: H8
    #define H8_BITMASK   _BV(PH8)
    #define H8_LETTER    H
    #define H8_NUMBER    8

    //Pxn: H9
    #define H9_BITMASK   _BV(PH9)
    #define H9_LETTER    H
    #define H9_NUMBER    9

    //Pxn: H10
    #define H10_BITMASK   _BV(PH10)
    #define H10_LETTER    H
    #define H10_NUMBER    10

    //Pxn: H12
    #define H12_BITMASK   _BV(PH12)
    #define H12_LETTER    H
    #define H12_NUMBER    12

    //Pxn: H13
    #define H13_BITMASK   _BV(PH13)
    #define H13_LETTER    H
    #define H13_NUMBER    13

    //Pxn: H14
    #define H14_BITMASK   _BV(PH14)
    #define H14_LETTER    H
    #define H14_NUMBER    14

    //Pxn: H15
    #define H15_BITMASK   _BV(PH15)
    #define H15_LETTER    H
    #define H15_NUMBER    15

    //Pxn: H16
    #define H16_BITMASK   _BV(PH16)
    #define H16_LETTER    H
    #define H16_NUMBER    16


//## Pin Letter:I

    //Pxn: I0
    #define I0_BITMASK   _BV(PI0)
    #define I0_LETTER    I
    #define I0_NUMBER    0

    //Pxn: I1
    #define I1_BITMASK   _BV(PI1)
    #define I1_LETTER    I
    #define I1_NUMBER    1

    //Pxn: I2
    #define I2_BITMASK   _BV(PI2)
    #define I2_LETTER    I
    #define I2_NUMBER    2

    //Pxn: I3
    #define I3_BITMASK   _BV(PI3)
    #define I3_LETTER    I
    #define I3_NUMBER    3

    //Pxn: I4
    #define I4_BITMASK   _BV(PI4)
    #define I4_LETTER    I
    #define I4_NUMBER    4

    //Pxn: I5
    #define I5_BITMASK   _BV(PI5)
    #define I5_LETTER    I
    #define I5_NUMBER    5

    //Pxn: I6
    #define I6_BITMASK   _BV(PI6)
    #define I6_LETTER    I
    #define I6_NUMBER    6

    //Pxn: I7
    #define I7_BITMASK   _BV(PI7)
    #define I7_LETTER    I
    #define I7_NUMBER    7

    //Pxn: I8
    #define I8_BITMASK   _BV(PI8)
    #define I8_LETTER    I
    #define I8_NUMBER    8

    //Pxn: I9
    #define I9_BITMASK   _BV(PI9)
    #define I9_LETTER    I
    #define I9_NUMBER    9

    //Pxn: I10
    #define I10_BITMASK   _BV(PI10)
    #define I10_LETTER    I
    #define I10_NUMBER    10

    //Pxn: I12
    #define I12_BITMASK   _BV(PI12)
    #define I12_LETTER    I
    #define I12_NUMBER    12

    //Pxn: I13
    #define I13_BITMASK   _BV(PI13)
    #define I13_LETTER    I
    #define I13_NUMBER    13

    //Pxn: I14
    #define I14_BITMASK   _BV(PI14)
    #define I14_LETTER    I
    #define I14_NUMBER    14

    //Pxn: I15
    #define I15_BITMASK   _BV(PI15)
    #define I15_LETTER    I
    #define I15_NUMBER    15

    //Pxn: I16
    #define I16_BITMASK   _BV(PI16)
    #define I16_LETTER    I
    #define I16_NUMBER    16


//## Pin Letter:J

    //Pxn: J0
    #define J0_BITMASK   _BV(PJ0)
    #define J0_LETTER    J
    #define J0_NUMBER    0

    //Pxn: J1
    #define J1_BITMASK   _BV(PJ1)
    #define J1_LETTER    J
    #define J1_NUMBER    1

    //Pxn: J2
    #define J2_BITMASK   _BV(PJ2)
    #define J2_LETTER    J
    #define J2_NUMBER    2

    //Pxn: J3
    #define J3_BITMASK   _BV(PJ3)
    #define J3_LETTER    J
    #define J3_NUMBER    3

    //Pxn: J4
    #define J4_BITMASK   _BV(PJ4)
    #define J4_LETTER    J
    #define J4_NUMBER    4

    //Pxn: J5
    #define J5_BITMASK   _BV(PJ5)
    #define J5_LETTER    J
    #define J5_NUMBER    5

    //Pxn: J6
    #define J6_BITMASK   _BV(PJ6)
    #define J6_LETTER    J
    #define J6_NUMBER    6

    //Pxn: J7
    #define J7_BITMASK   _BV(PJ7)
    #define J7_LETTER    J
    #define J7_NUMBER    7

    //Pxn: J8
    #define J8_BITMASK   _BV(PJ8)
    #define J8_LETTER    J
    #define J8_NUMBER    8

    //Pxn: J9
    #define J9_BITMASK   _BV(PJ9)
    #define J9_LETTER    J
    #define J9_NUMBER    9

    //Pxn: J10
    #define J10_BITMASK   _BV(PJ10)
    #define J10_LETTER    J
    #define J10_NUMBER    10

    //Pxn: J12
    #define J12_BITMASK   _BV(PJ12)
    #define J12_LETTER    J
    #define J12_NUMBER    12

    //Pxn: J13
    #define J13_BITMASK   _BV(PJ13)
    #define J13_LETTER    J
    #define J13_NUMBER    13

    //Pxn: J14
    #define J14_BITMASK   _BV(PJ14)
    #define J14_LETTER    J
    #define J14_NUMBER    14

    //Pxn: J15
    #define J15_BITMASK   _BV(PJ15)
    #define J15_LETTER    J
    #define J15_NUMBER    15

    //Pxn: J16
    #define J16_BITMASK   _BV(PJ16)
    #define J16_LETTER    J
    #define J16_NUMBER    16


//## Pin Letter:K

    //Pxn: K0
    #define K0_BITMASK   _BV(PK0)
    #define K0_LETTER    K
    #define K0_NUMBER    0

    //Pxn: K1
    #define K1_BITMASK   _BV(PK1)
    #define K1_LETTER    K
    #define K1_NUMBER    1

    //Pxn: K2
    #define K2_BITMASK   _BV(PK2)
    #define K2_LETTER    K
    #define K2_NUMBER    2

    //Pxn: K3
    #define K3_BITMASK   _BV(PK3)
    #define K3_LETTER    K
    #define K3_NUMBER    3

    //Pxn: K4
    #define K4_BITMASK   _BV(PK4)
    #define K4_LETTER    K
    #define K4_NUMBER    4

    //Pxn: K5
    #define K5_BITMASK   _BV(PK5)
    #define K5_LETTER    K
    #define K5_NUMBER    5

    //Pxn: K6
    #define K6_BITMASK   _BV(PK6)
    #define K6_LETTER    K
    #define K6_NUMBER    6

    //Pxn: K7
    #define K7_BITMASK   _BV(PK7)
    #define K7_LETTER    K
    #define K7_NUMBER    7

    //Pxn: K8
    #define K8_BITMASK   _BV(PK8)
    #define K8_LETTER    K
    #define K8_NUMBER    8

    //Pxn: K9
    #define K9_BITMASK   _BV(PK9)
    #define K9_LETTER    K
    #define K9_NUMBER    9

    //Pxn: K10
    #define K10_BITMASK   _BV(PK10)
    #define K10_LETTER    K
    #define K10_NUMBER    10

    //Pxn: K12
    #define K12_BITMASK   _BV(PK12)
    #define K12_LETTER    K
    #define K12_NUMBER    12

    //Pxn: K13
    #define K13_BITMASK   _BV(PK13)
    #define K13_LETTER    K
    #define K13_NUMBER    13

    //Pxn: K14
    #define K14_BITMASK   _BV(PK14)
    #define K14_LETTER    K
    #define K14_NUMBER    14

    //Pxn: K15
    #define K15_BITMASK   _BV(PK15)
    #define K15_LETTER    K
    #define K15_NUMBER    15

    //Pxn: K16
    #define K16_BITMASK   _BV(PK16)
    #define K16_LETTER    K
    #define K16_NUMBER    16


//## Pin Letter:L

    //Pxn: L0
    #define L0_BITMASK   _BV(PL0)
    #define L0_LETTER    L
    #define L0_NUMBER    0

    //Pxn: L1
    #define L1_BITMASK   _BV(PL1)
    #define L1_LETTER    L
    #define L1_NUMBER    1

    //Pxn: L2
    #define L2_BITMASK   _BV(PL2)
    #define L2_LETTER    L
    #define L2_NUMBER    2

    //Pxn: L3
    #define L3_BITMASK   _BV(PL3)
    #define L3_LETTER    L
    #define L3_NUMBER    3

    //Pxn: L4
    #define L4_BITMASK   _BV(PL4)
    #define L4_LETTER    L
    #define L4_NUMBER    4

    //Pxn: L5
    #define L5_BITMASK   _BV(PL5)
    #define L5_LETTER    L
    #define L5_NUMBER    5

    //Pxn: L6
    #define L6_BITMASK   _BV(PL6)
    #define L6_LETTER    L
    #define L6_NUMBER    6

    //Pxn: L7
    #define L7_BITMASK   _BV(PL7)
    #define L7_LETTER    L
    #define L7_NUMBER    7

    //Pxn: L8
    #define L8_BITMASK   _BV(PL8)
    #define L8_LETTER    L
    #define L8_NUMBER    8

    //Pxn: L9
    #define L9_BITMASK   _BV(PL9)
    #define L9_LETTER    L
    #define L9_NUMBER    9

    //Pxn: L10
    #define L10_BITMASK   _BV(PL10)
    #define L10_LETTER    L
    #define L10_NUMBER    10

    //Pxn: L12
    #define L12_BITMASK   _BV(PL12)
    #define L12_LETTER    L
    #define L12_NUMBER    12

    //Pxn: L13
    #define L13_BITMASK   _BV(PL13)
    #define L13_LETTER    L
    #define L13_NUMBER    13

    //Pxn: L14
    #define L14_BITMASK   _BV(PL14)
    #define L14_LETTER    L
    #define L14_NUMBER    14

    //Pxn: L15
    #define L15_BITMASK   _BV(PL15)
    #define L15_LETTER    L
    #define L15_NUMBER    15

    //Pxn: L16
    #define L16_BITMASK   _BV(PL16)
    #define L16_LETTER    L
    #define L16_NUMBER    16


//## Pin Letter:M

    //Pxn: M0
    #define M0_BITMASK   _BV(PM0)
    #define M0_LETTER    M
    #define M0_NUMBER    0

    //Pxn: M1
    #define M1_BITMASK   _BV(PM1)
    #define M1_LETTER    M
    #define M1_NUMBER    1

    //Pxn: M2
    #define M2_BITMASK   _BV(PM2)
    #define M2_LETTER    M
    #define M2_NUMBER    2

    //Pxn: M3
    #define M3_BITMASK   _BV(PM3)
    #define M3_LETTER    M
    #define M3_NUMBER    3

    //Pxn: M4
    #define M4_BITMASK   _BV(PM4)
    #define M4_LETTER    M
    #define M4_NUMBER    4

    //Pxn: M5
    #define M5_BITMASK   _BV(PM5)
    #define M5_LETTER    M
    #define M5_NUMBER    5

    //Pxn: M6
    #define M6_BITMASK   _BV(PM6)
    #define M6_LETTER    M
    #define M6_NUMBER    6

    //Pxn: M7
    #define M7_BITMASK   _BV(PM7)
    #define M7_LETTER    M
    #define M7_NUMBER    7

    //Pxn: M8
    #define M8_BITMASK   _BV(PM8)
    #define M8_LETTER    M
    #define M8_NUMBER    8

    //Pxn: M9
    #define M9_BITMASK   _BV(PM9)
    #define M9_LETTER    M
    #define M9_NUMBER    9

    //Pxn: M10
    #define M10_BITMASK   _BV(PM10)
    #define M10_LETTER    M
    #define M10_NUMBER    10

    //Pxn: M12
    #define M12_BITMASK   _BV(PM12)
    #define M12_LETTER    M
    #define M12_NUMBER    12

    //Pxn: M13
    #define M13_BITMASK   _BV(PM13)
    #define M13_LETTER    M
    #define M13_NUMBER    13

    //Pxn: M14
    #define M14_BITMASK   _BV(PM14)
    #define M14_LETTER    M
    #define M14_NUMBER    14

    //Pxn: M15
    #define M15_BITMASK   _BV(PM15)
    #define M15_LETTER    M
    #define M15_NUMBER    15

    //Pxn: M16
    #define M16_BITMASK   _BV(PM16)
    #define M16_LETTER    M
    #define M16_NUMBER    16


//## Pin Letter:N

    //Pxn: N0
    #define N0_BITMASK   _BV(PN0)
    #define N0_LETTER    N
    #define N0_NUMBER    0

    //Pxn: N1
    #define N1_BITMASK   _BV(PN1)
    #define N1_LETTER    N
    #define N1_NUMBER    1

    //Pxn: N2
    #define N2_BITMASK   _BV(PN2)
    #define N2_LETTER    N
    #define N2_NUMBER    2

    //Pxn: N3
    #define N3_BITMASK   _BV(PN3)
    #define N3_LETTER    N
    #define N3_NUMBER    3

    //Pxn: N4
    #define N4_BITMASK   _BV(PN4)
    #define N4_LETTER    N
    #define N4_NUMBER    4

    //Pxn: N5
    #define N5_BITMASK   _BV(PN5)
    #define N5_LETTER    N
    #define N5_NUMBER    5

    //Pxn: N6
    #define N6_BITMASK   _BV(PN6)
    #define N6_LETTER    N
    #define N6_NUMBER    6

    //Pxn: N7
    #define N7_BITMASK   _BV(PN7)
    #define N7_LETTER    N
    #define N7_NUMBER    7

    //Pxn: N8
    #define N8_BITMASK   _BV(PN8)
    #define N8_LETTER    N
    #define N8_NUMBER    8

    //Pxn: N9
    #define N9_BITMASK   _BV(PN9)
    #define N9_LETTER    N
    #define N9_NUMBER    9

    //Pxn: N10
    #define N10_BITMASK   _BV(PN10)
    #define N10_LETTER    N
    #define N10_NUMBER    10

    //Pxn: N12
    #define N12_BITMASK   _BV(PN12)
    #define N12_LETTER    N
    #define N12_NUMBER    12

    //Pxn: N13
    #define N13_BITMASK   _BV(PN13)
    #define N13_LETTER    N
    #define N13_NUMBER    13

    //Pxn: N14
    #define N14_BITMASK   _BV(PN14)
    #define N14_LETTER    N
    #define N14_NUMBER    14

    //Pxn: N15
    #define N15_BITMASK   _BV(PN15)
    #define N15_LETTER    N
    #define N15_NUMBER    15

    //Pxn: N16
    #define N16_BITMASK   _BV(PN16)
    #define N16_LETTER    N
    #define N16_NUMBER    16


//## Pin Letter:O

    //Pxn: O0
    #define O0_BITMASK   _BV(PO0)
    #define O0_LETTER    O
    #define O0_NUMBER    0

    //Pxn: O1
    #define O1_BITMASK   _BV(PO1)
    #define O1_LETTER    O
    #define O1_NUMBER    1

    //Pxn: O2
    #define O2_BITMASK   _BV(PO2)
    #define O2_LETTER    O
    #define O2_NUMBER    2

    //Pxn: O3
    #define O3_BITMASK   _BV(PO3)
    #define O3_LETTER    O
    #define O3_NUMBER    3

    //Pxn: O4
    #define O4_BITMASK   _BV(PO4)
    #define O4_LETTER    O
    #define O4_NUMBER    4

    //Pxn: O5
    #define O5_BITMASK   _BV(PO5)
    #define O5_LETTER    O
    #define O5_NUMBER    5

    //Pxn: O6
    #define O6_BITMASK   _BV(PO6)
    #define O6_LETTER    O
    #define O6_NUMBER    6

    //Pxn: O7
    #define O7_BITMASK   _BV(PO7)
    #define O7_LETTER    O
    #define O7_NUMBER    7

    //Pxn: O8
    #define O8_BITMASK   _BV(PO8)
    #define O8_LETTER    O
    #define O8_NUMBER    8

    //Pxn: O9
    #define O9_BITMASK   _BV(PO9)
    #define O9_LETTER    O
    #define O9_NUMBER    9

    //Pxn: O10
    #define O10_BITMASK   _BV(PO10)
    #define O10_LETTER    O
    #define O10_NUMBER    10

    //Pxn: O12
    #define O12_BITMASK   _BV(PO12)
    #define O12_LETTER    O
    #define O12_NUMBER    12

    //Pxn: O13
    #define O13_BITMASK   _BV(PO13)
    #define O13_LETTER    O
    #define O13_NUMBER    13

    //Pxn: O14
    #define O14_BITMASK   _BV(PO14)
    #define O14_LETTER    O
    #define O14_NUMBER    14

    //Pxn: O15
    #define O15_BITMASK   _BV(PO15)
    #define O15_LETTER    O
    #define O15_NUMBER    15

    //Pxn: O16
    #define O16_BITMASK   _BV(PO16)
    #define O16_LETTER    O
    #define O16_NUMBER    16


//## Pin Letter:P

    //Pxn: P0
    #define P0_BITMASK   _BV(PP0)
    #define P0_LETTER    P
    #define P0_NUMBER    0

    //Pxn: P1
    #define P1_BITMASK   _BV(PP1)
    #define P1_LETTER    P
    #define P1_NUMBER    1

    //Pxn: P2
    #define P2_BITMASK   _BV(PP2)
    #define P2_LETTER    P
    #define P2_NUMBER    2

    //Pxn: P3
    #define P3_BITMASK   _BV(PP3)
    #define P3_LETTER    P
    #define P3_NUMBER    3

    //Pxn: P4
    #define P4_BITMASK   _BV(PP4)
    #define P4_LETTER    P
    #define P4_NUMBER    4

    //Pxn: P5
    #define P5_BITMASK   _BV(PP5)
    #define P5_LETTER    P
    #define P5_NUMBER    5

    //Pxn: P6
    #define P6_BITMASK   _BV(PP6)
    #define P6_LETTER    P
    #define P6_NUMBER    6

    //Pxn: P7
    #define P7_BITMASK   _BV(PP7)
    #define P7_LETTER    P
    #define P7_NUMBER    7

    //Pxn: P8
    #define P8_BITMASK   _BV(PP8)
    #define P8_LETTER    P
    #define P8_NUMBER    8

    //Pxn: P9
    #define P9_BITMASK   _BV(PP9)
    #define P9_LETTER    P
    #define P9_NUMBER    9

    //Pxn: P10
    #define P10_BITMASK   _BV(PP10)
    #define P10_LETTER    P
    #define P10_NUMBER    10

    //Pxn: P12
    #define P12_BITMASK   _BV(PP12)
    #define P12_LETTER    P
    #define P12_NUMBER    12

    //Pxn: P13
    #define P13_BITMASK   _BV(PP13)
    #define P13_LETTER    P
    #define P13_NUMBER    13

    //Pxn: P14
    #define P14_BITMASK   _BV(PP14)
    #define P14_LETTER    P
    #define P14_NUMBER    14

    //Pxn: P15
    #define P15_BITMASK   _BV(PP15)
    #define P15_LETTER    P
    #define P15_NUMBER    15

    //Pxn: P16
    #define P16_BITMASK   _BV(PP16)
    #define P16_LETTER    P
    #define P16_NUMBER    16


//## Pin Letter:Q

    //Pxn: Q0
    #define Q0_BITMASK   _BV(PQ0)
    #define Q0_LETTER    Q
    #define Q0_NUMBER    0

    //Pxn: Q1
    #define Q1_BITMASK   _BV(PQ1)
    #define Q1_LETTER    Q
    #define Q1_NUMBER    1

    //Pxn: Q2
    #define Q2_BITMASK   _BV(PQ2)
    #define Q2_LETTER    Q
    #define Q2_NUMBER    2

    //Pxn: Q3
    #define Q3_BITMASK   _BV(PQ3)
    #define Q3_LETTER    Q
    #define Q3_NUMBER    3

    //Pxn: Q4
    #define Q4_BITMASK   _BV(PQ4)
    #define Q4_LETTER    Q
    #define Q4_NUMBER    4

    //Pxn: Q5
    #define Q5_BITMASK   _BV(PQ5)
    #define Q5_LETTER    Q
    #define Q5_NUMBER    5

    //Pxn: Q6
    #define Q6_BITMASK   _BV(PQ6)
    #define Q6_LETTER    Q
    #define Q6_NUMBER    6

    //Pxn: Q7
    #define Q7_BITMASK   _BV(PQ7)
    #define Q7_LETTER    Q
    #define Q7_NUMBER    7

    //Pxn: Q8
    #define Q8_BITMASK   _BV(PQ8)
    #define Q8_LETTER    Q
    #define Q8_NUMBER    8

    //Pxn: Q9
    #define Q9_BITMASK   _BV(PQ9)
    #define Q9_LETTER    Q
    #define Q9_NUMBER    9

    //Pxn: Q10
    #define Q10_BITMASK   _BV(PQ10)
    #define Q10_LETTER    Q
    #define Q10_NUMBER    10

    //Pxn: Q12
    #define Q12_BITMASK   _BV(PQ12)
    #define Q12_LETTER    Q
    #define Q12_NUMBER    12

    //Pxn: Q13
    #define Q13_BITMASK   _BV(PQ13)
    #define Q13_LETTER    Q
    #define Q13_NUMBER    13

    //Pxn: Q14
    #define Q14_BITMASK   _BV(PQ14)
    #define Q14_LETTER    Q
    #define Q14_NUMBER    14

    //Pxn: Q15
    #define Q15_BITMASK   _BV(PQ15)
    #define Q15_LETTER    Q
    #define Q15_NUMBER    15

    //Pxn: Q16
    #define Q16_BITMASK   _BV(PQ16)
    #define Q16_LETTER    Q
    #define Q16_NUMBER    16


//## Pin Letter:R

    //Pxn: R0
    #define R0_BITMASK   _BV(PR0)
    #define R0_LETTER    R
    #define R0_NUMBER    0

    //Pxn: R1
    #define R1_BITMASK   _BV(PR1)
    #define R1_LETTER    R
    #define R1_NUMBER    1

    //Pxn: R2
    #define R2_BITMASK   _BV(PR2)
    #define R2_LETTER    R
    #define R2_NUMBER    2

    //Pxn: R3
    #define R3_BITMASK   _BV(PR3)
    #define R3_LETTER    R
    #define R3_NUMBER    3

    //Pxn: R4
    #define R4_BITMASK   _BV(PR4)
    #define R4_LETTER    R
    #define R4_NUMBER    4

    //Pxn: R5
    #define R5_BITMASK   _BV(PR5)
    #define R5_LETTER    R
    #define R5_NUMBER    5

    //Pxn: R6
    #define R6_BITMASK   _BV(PR6)
    #define R6_LETTER    R
    #define R6_NUMBER    6

    //Pxn: R7
    #define R7_BITMASK   _BV(PR7)
    #define R7_LETTER    R
    #define R7_NUMBER    7

    //Pxn: R8
    #define R8_BITMASK   _BV(PR8)
    #define R8_LETTER    R
    #define R8_NUMBER    8

    //Pxn: R9
    #define R9_BITMASK   _BV(PR9)
    #define R9_LETTER    R
    #define R9_NUMBER    9

    //Pxn: R10
    #define R10_BITMASK   _BV(PR10)
    #define R10_LETTER    R
    #define R10_NUMBER    10

    //Pxn: R12
    #define R12_BITMASK   _BV(PR12)
    #define R12_LETTER    R
    #define R12_NUMBER    12

    //Pxn: R13
    #define R13_BITMASK   _BV(PR13)
    #define R13_LETTER    R
    #define R13_NUMBER    13

    //Pxn: R14
    #define R14_BITMASK   _BV(PR14)
    #define R14_LETTER    R
    #define R14_NUMBER    14

    //Pxn: R15
    #define R15_BITMASK   _BV(PR15)
    #define R15_LETTER    R
    #define R15_NUMBER    15

    //Pxn: R16
    #define R16_BITMASK   _BV(PR16)
    #define R16_LETTER    R
    #define R16_NUMBER    16


//## Pin Letter:S

    //Pxn: S0
    #define S0_BITMASK   _BV(PS0)
    #define S0_LETTER    S
    #define S0_NUMBER    0

    //Pxn: S1
    #define S1_BITMASK   _BV(PS1)
    #define S1_LETTER    S
    #define S1_NUMBER    1

    //Pxn: S2
    #define S2_BITMASK   _BV(PS2)
    #define S2_LETTER    S
    #define S2_NUMBER    2

    //Pxn: S3
    #define S3_BITMASK   _BV(PS3)
    #define S3_LETTER    S
    #define S3_NUMBER    3

    //Pxn: S4
    #define S4_BITMASK   _BV(PS4)
    #define S4_LETTER    S
    #define S4_NUMBER    4

    //Pxn: S5
    #define S5_BITMASK   _BV(PS5)
    #define S5_LETTER    S
    #define S5_NUMBER    5

    //Pxn: S6
    #define S6_BITMASK   _BV(PS6)
    #define S6_LETTER    S
    #define S6_NUMBER    6

    //Pxn: S7
    #define S7_BITMASK   _BV(PS7)
    #define S7_LETTER    S
    #define S7_NUMBER    7

    //Pxn: S8
    #define S8_BITMASK   _BV(PS8)
    #define S8_LETTER    S
    #define S8_NUMBER    8

    //Pxn: S9
    #define S9_BITMASK   _BV(PS9)
    #define S9_LETTER    S
    #define S9_NUMBER    9

    //Pxn: S10
    #define S10_BITMASK   _BV(PS10)
    #define S10_LETTER    S
    #define S10_NUMBER    10

    //Pxn: S12
    #define S12_BITMASK   _BV(PS12)
    #define S12_LETTER    S
    #define S12_NUMBER    12

    //Pxn: S13
    #define S13_BITMASK   _BV(PS13)
    #define S13_LETTER    S
    #define S13_NUMBER    13

    //Pxn: S14
    #define S14_BITMASK   _BV(PS14)
    #define S14_LETTER    S
    #define S14_NUMBER    14

    //Pxn: S15
    #define S15_BITMASK   _BV(PS15)
    #define S15_LETTER    S
    #define S15_NUMBER    15

    //Pxn: S16
    #define S16_BITMASK   _BV(PS16)
    #define S16_LETTER    S
    #define S16_NUMBER    16


//## Pin Letter:T

    //Pxn: T0
    #define T0_BITMASK   _BV(PT0)
    #define T0_LETTER    T
    #define T0_NUMBER    0

    //Pxn: T1
    #define T1_BITMASK   _BV(PT1)
    #define T1_LETTER    T
    #define T1_NUMBER    1

    //Pxn: T2
    #define T2_BITMASK   _BV(PT2)
    #define T2_LETTER    T
    #define T2_NUMBER    2

    //Pxn: T3
    #define T3_BITMASK   _BV(PT3)
    #define T3_LETTER    T
    #define T3_NUMBER    3

    //Pxn: T4
    #define T4_BITMASK   _BV(PT4)
    #define T4_LETTER    T
    #define T4_NUMBER    4

    //Pxn: T5
    #define T5_BITMASK   _BV(PT5)
    #define T5_LETTER    T
    #define T5_NUMBER    5

    //Pxn: T6
    #define T6_BITMASK   _BV(PT6)
    #define T6_LETTER    T
    #define T6_NUMBER    6

    //Pxn: T7
    #define T7_BITMASK   _BV(PT7)
    #define T7_LETTER    T
    #define T7_NUMBER    7

    //Pxn: T8
    #define T8_BITMASK   _BV(PT8)
    #define T8_LETTER    T
    #define T8_NUMBER    8

    //Pxn: T9
    #define T9_BITMASK   _BV(PT9)
    #define T9_LETTER    T
    #define T9_NUMBER    9

    //Pxn: T10
    #define T10_BITMASK   _BV(PT10)
    #define T10_LETTER    T
    #define T10_NUMBER    10

    //Pxn: T12
    #define T12_BITMASK   _BV(PT12)
    #define T12_LETTER    T
    #define T12_NUMBER    12

    //Pxn: T13
    #define T13_BITMASK   _BV(PT13)
    #define T13_LETTER    T
    #define T13_NUMBER    13

    //Pxn: T14
    #define T14_BITMASK   _BV(PT14)
    #define T14_LETTER    T
    #define T14_NUMBER    14

    //Pxn: T15
    #define T15_BITMASK   _BV(PT15)
    #define T15_LETTER    T
    #define T15_NUMBER    15

    //Pxn: T16
    #define T16_BITMASK   _BV(PT16)
    #define T16_LETTER    T
    #define T16_NUMBER    16


//## Pin Letter:U

    //Pxn: U0
    #define U0_BITMASK   _BV(PU0)
    #define U0_LETTER    U
    #define U0_NUMBER    0

    //Pxn: U1
    #define U1_BITMASK   _BV(PU1)
    #define U1_LETTER    U
    #define U1_NUMBER    1

    //Pxn: U2
    #define U2_BITMASK   _BV(PU2)
    #define U2_LETTER    U
    #define U2_NUMBER    2

    //Pxn: U3
    #define U3_BITMASK   _BV(PU3)
    #define U3_LETTER    U
    #define U3_NUMBER    3

    //Pxn: U4
    #define U4_BITMASK   _BV(PU4)
    #define U4_LETTER    U
    #define U4_NUMBER    4

    //Pxn: U5
    #define U5_BITMASK   _BV(PU5)
    #define U5_LETTER    U
    #define U5_NUMBER    5

    //Pxn: U6
    #define U6_BITMASK   _BV(PU6)
    #define U6_LETTER    U
    #define U6_NUMBER    6

    //Pxn: U7
    #define U7_BITMASK   _BV(PU7)
    #define U7_LETTER    U
    #define U7_NUMBER    7

    //Pxn: U8
    #define U8_BITMASK   _BV(PU8)
    #define U8_LETTER    U
    #define U8_NUMBER    8

    //Pxn: U9
    #define U9_BITMASK   _BV(PU9)
    #define U9_LETTER    U
    #define U9_NUMBER    9

    //Pxn: U10
    #define U10_BITMASK   _BV(PU10)
    #define U10_LETTER    U
    #define U10_NUMBER    10

    //Pxn: U12
    #define U12_BITMASK   _BV(PU12)
    #define U12_LETTER    U
    #define U12_NUMBER    12

    //Pxn: U13
    #define U13_BITMASK   _BV(PU13)
    #define U13_LETTER    U
    #define U13_NUMBER    13

    //Pxn: U14
    #define U14_BITMASK   _BV(PU14)
    #define U14_LETTER    U
    #define U14_NUMBER    14

    //Pxn: U15
    #define U15_BITMASK   _BV(PU15)
    #define U15_LETTER    U
    #define U15_NUMBER    15

    //Pxn: U16
    #define U16_BITMASK   _BV(PU16)
    #define U16_LETTER    U
    #define U16_NUMBER    16


//## Pin Letter:V

    //Pxn: V0
    #define V0_BITMASK   _BV(PV0)
    #define V0_LETTER    V
    #define V0_NUMBER    0

    //Pxn: V1
    #define V1_BITMASK   _BV(PV1)
    #define V1_LETTER    V
    #define V1_NUMBER    1

    //Pxn: V2
    #define V2_BITMASK   _BV(PV2)
    #define V2_LETTER    V
    #define V2_NUMBER    2

    //Pxn: V3
    #define V3_BITMASK   _BV(PV3)
    #define V3_LETTER    V
    #define V3_NUMBER    3

    //Pxn: V4
    #define V4_BITMASK   _BV(PV4)
    #define V4_LETTER    V
    #define V4_NUMBER    4

    //Pxn: V5
    #define V5_BITMASK   _BV(PV5)
    #define V5_LETTER    V
    #define V5_NUMBER    5

    //Pxn: V6
    #define V6_BITMASK   _BV(PV6)
    #define V6_LETTER    V
    #define V6_NUMBER    6

    //Pxn: V7
    #define V7_BITMASK   _BV(PV7)
    #define V7_LETTER    V
    #define V7_NUMBER    7

    //Pxn: V8
    #define V8_BITMASK   _BV(PV8)
    #define V8_LETTER    V
    #define V8_NUMBER    8

    //Pxn: V9
    #define V9_BITMASK   _BV(PV9)
    #define V9_LETTER    V
    #define V9_NUMBER    9

    //Pxn: V10
    #define V10_BITMASK   _BV(PV10)
    #define V10_LETTER    V
    #define V10_NUMBER    10

    //Pxn: V12
    #define V12_BITMASK   _BV(PV12)
    #define V12_LETTER    V
    #define V12_NUMBER    12

    //Pxn: V13
    #define V13_BITMASK   _BV(PV13)
    #define V13_LETTER    V
    #define V13_NUMBER    13

    //Pxn: V14
    #define V14_BITMASK   _BV(PV14)
    #define V14_LETTER    V
    #define V14_NUMBER    14

    //Pxn: V15
    #define V15_BITMASK   _BV(PV15)
    #define V15_LETTER    V
    #define V15_NUMBER    15

    //Pxn: V16
    #define V16_BITMASK   _BV(PV16)
    #define V16_LETTER    V
    #define V16_NUMBER    16


//## Pin Letter:W

    //Pxn: W0
    #define W0_BITMASK   _BV(PW0)
    #define W0_LETTER    W
    #define W0_NUMBER    0

    //Pxn: W1
    #define W1_BITMASK   _BV(PW1)
    #define W1_LETTER    W
    #define W1_NUMBER    1

    //Pxn: W2
    #define W2_BITMASK   _BV(PW2)
    #define W2_LETTER    W
    #define W2_NUMBER    2

    //Pxn: W3
    #define W3_BITMASK   _BV(PW3)
    #define W3_LETTER    W
    #define W3_NUMBER    3

    //Pxn: W4
    #define W4_BITMASK   _BV(PW4)
    #define W4_LETTER    W
    #define W4_NUMBER    4

    //Pxn: W5
    #define W5_BITMASK   _BV(PW5)
    #define W5_LETTER    W
    #define W5_NUMBER    5

    //Pxn: W6
    #define W6_BITMASK   _BV(PW6)
    #define W6_LETTER    W
    #define W6_NUMBER    6

    //Pxn: W7
    #define W7_BITMASK   _BV(PW7)
    #define W7_LETTER    W
    #define W7_NUMBER    7

    //Pxn: W8
    #define W8_BITMASK   _BV(PW8)
    #define W8_LETTER    W
    #define W8_NUMBER    8

    //Pxn: W9
    #define W9_BITMASK   _BV(PW9)
    #define W9_LETTER    W
    #define W9_NUMBER    9

    //Pxn: W10
    #define W10_BITMASK   _BV(PW10)
    #define W10_LETTER    W
    #define W10_NUMBER    10

    //Pxn: W12
    #define W12_BITMASK   _BV(PW12)
    #define W12_LETTER    W
    #define W12_NUMBER    12

    //Pxn: W13
    #define W13_BITMASK   _BV(PW13)
    #define W13_LETTER    W
    #define W13_NUMBER    13

    //Pxn: W14
    #define W14_BITMASK   _BV(PW14)
    #define W14_LETTER    W
    #define W14_NUMBER    14

    //Pxn: W15
    #define W15_BITMASK   _BV(PW15)
    #define W15_LETTER    W
    #define W15_NUMBER    15

    //Pxn: W16
    #define W16_BITMASK   _BV(PW16)
    #define W16_LETTER    W
    #define W16_NUMBER    16


//## Pin Letter:X

    //Pxn: X0
    #define X0_BITMASK   _BV(PX0)
    #define X0_LETTER    X
    #define X0_NUMBER    0

    //Pxn: X1
    #define X1_BITMASK   _BV(PX1)
    #define X1_LETTER    X
    #define X1_NUMBER    1

    //Pxn: X2
    #define X2_BITMASK   _BV(PX2)
    #define X2_LETTER    X
    #define X2_NUMBER    2

    //Pxn: X3
    #define X3_BITMASK   _BV(PX3)
    #define X3_LETTER    X
    #define X3_NUMBER    3

    //Pxn: X4
    #define X4_BITMASK   _BV(PX4)
    #define X4_LETTER    X
    #define X4_NUMBER    4

    //Pxn: X5
    #define X5_BITMASK   _BV(PX5)
    #define X5_LETTER    X
    #define X5_NUMBER    5

    //Pxn: X6
    #define X6_BITMASK   _BV(PX6)
    #define X6_LETTER    X
    #define X6_NUMBER    6

    //Pxn: X7
    #define X7_BITMASK   _BV(PX7)
    #define X7_LETTER    X
    #define X7_NUMBER    7

    //Pxn: X8
    #define X8_BITMASK   _BV(PX8)
    #define X8_LETTER    X
    #define X8_NUMBER    8

    //Pxn: X9
    #define X9_BITMASK   _BV(PX9)
    #define X9_LETTER    X
    #define X9_NUMBER    9

    //Pxn: X10
    #define X10_BITMASK   _BV(PX10)
    #define X10_LETTER    X
    #define X10_NUMBER    10

    //Pxn: X12
    #define X12_BITMASK   _BV(PX12)
    #define X12_LETTER    X
    #define X12_NUMBER    12

    //Pxn: X13
    #define X13_BITMASK   _BV(PX13)
    #define X13_LETTER    X
    #define X13_NUMBER    13

    //Pxn: X14
    #define X14_BITMASK   _BV(PX14)
    #define X14_LETTER    X
    #define X14_NUMBER    14

    //Pxn: X15
    #define X15_BITMASK   _BV(PX15)
    #define X15_LETTER    X
    #define X15_NUMBER    15

    //Pxn: X16
    #define X16_BITMASK   _BV(PX16)
    #define X16_LETTER    X
    #define X16_NUMBER    16


//## Pin Letter:Y

    //Pxn: Y0
    #define Y0_BITMASK   _BV(PY0)
    #define Y0_LETTER    Y
    #define Y0_NUMBER    0

    //Pxn: Y1
    #define Y1_BITMASK   _BV(PY1)
    #define Y1_LETTER    Y
    #define Y1_NUMBER    1

    //Pxn: Y2
    #define Y2_BITMASK   _BV(PY2)
    #define Y2_LETTER    Y
    #define Y2_NUMBER    2

    //Pxn: Y3
    #define Y3_BITMASK   _BV(PY3)
    #define Y3_LETTER    Y
    #define Y3_NUMBER    3

    //Pxn: Y4
    #define Y4_BITMASK   _BV(PY4)
    #define Y4_LETTER    Y
    #define Y4_NUMBER    4

    //Pxn: Y5
    #define Y5_BITMASK   _BV(PY5)
    #define Y5_LETTER    Y
    #define Y5_NUMBER    5

    //Pxn: Y6
    #define Y6_BITMASK   _BV(PY6)
    #define Y6_LETTER    Y
    #define Y6_NUMBER    6

    //Pxn: Y7
    #define Y7_BITMASK   _BV(PY7)
    #define Y7_LETTER    Y
    #define Y7_NUMBER    7

    //Pxn: Y8
    #define Y8_BITMASK   _BV(PY8)
    #define Y8_LETTER    Y
    #define Y8_NUMBER    8

    //Pxn: Y9
    #define Y9_BITMASK   _BV(PY9)
    #define Y9_LETTER    Y
    #define Y9_NUMBER    9

    //Pxn: Y10
    #define Y10_BITMASK   _BV(PY10)
    #define Y10_LETTER    Y
    #define Y10_NUMBER    10

    //Pxn: Y12
    #define Y12_BITMASK   _BV(PY12)
    #define Y12_LETTER    Y
    #define Y12_NUMBER    12

    //Pxn: Y13
    #define Y13_BITMASK   _BV(PY13)
    #define Y13_LETTER    Y
    #define Y13_NUMBER    13

    //Pxn: Y14
    #define Y14_BITMASK   _BV(PY14)
    #define Y14_LETTER    Y
    #define Y14_NUMBER    14

    //Pxn: Y15
    #define Y15_BITMASK   _BV(PY15)
    #define Y15_LETTER    Y
    #define Y15_NUMBER    15

    //Pxn: Y16
    #define Y16_BITMASK   _BV(PY16)
    #define Y16_LETTER    Y
    #define Y16_NUMBER    16


//## Pin Letter:Z

    //Pxn: Z0
    #define Z0_BITMASK   _BV(PZ0)
    #define Z0_LETTER    Z
    #define Z0_NUMBER    0

    //Pxn: Z1
    #define Z1_BITMASK   _BV(PZ1)
    #define Z1_LETTER    Z
    #define Z1_NUMBER    1

    //Pxn: Z2
    #define Z2_BITMASK   _BV(PZ2)
    #define Z2_LETTER    Z
    #define Z2_NUMBER    2

    //Pxn: Z3
    #define Z3_BITMASK   _BV(PZ3)
    #define Z3_LETTER    Z
    #define Z3_NUMBER    3

    //Pxn: Z4
    #define Z4_BITMASK   _BV(PZ4)
    #define Z4_LETTER    Z
    #define Z4_NUMBER    4

    //Pxn: Z5
    #define Z5_BITMASK   _BV(PZ5)
    #define Z5_LETTER    Z
    #define Z5_NUMBER    5

    //Pxn: Z6
    #define Z6_BITMASK   _BV(PZ6)
    #define Z6_LETTER    Z
    #define Z6_NUMBER    6

    //Pxn: Z7
    #define Z7_BITMASK   _BV(PZ7)
    #define Z7_LETTER    Z
    #define Z7_NUMBER    7

    //Pxn: Z8
    #define Z8_BITMASK   _BV(PZ8)
    #define Z8_LETTER    Z
    #define Z8_NUMBER    8

    //Pxn: Z9
    #define Z9_BITMASK   _BV(PZ9)
    #define Z9_LETTER    Z
    #define Z9_NUMBER    9

    //Pxn: Z10
    #define Z10_BITMASK   _BV(PZ10)
    #define Z10_LETTER    Z
    #define Z10_NUMBER    10

    //Pxn: Z12
    #define Z12_BITMASK   _BV(PZ12)
    #define Z12_LETTER    Z
    #define Z12_NUMBER    12

    //Pxn: Z13
    #define Z13_BITMASK   _BV(PZ13)
    #define Z13_LETTER    Z
    #define Z13_NUMBER    13

    //Pxn: Z14
    #define Z14_BITMASK   _BV(PZ14)
    #define Z14_LETTER    Z
    #define Z14_NUMBER    14

    //Pxn: Z15
    #define Z15_BITMASK   _BV(PZ15)
    #define Z15_LETTER    Z
    #define Z15_NUMBER    15

    //Pxn: Z16
    #define Z16_BITMASK   _BV(PZ16)
    #define Z16_LETTER    Z
    #define Z16_NUMBER    16




// ~~~~~~~~~ EXAMPLE ~~~~~~~~~~~
/*


// Example blink code for Arduino UNO 16MHz (atmega328p)
#define F_CPU 16000000UL // For delay.h to function
#include <avr/io.h> // PIN, DDR, PORT defs
#include <util/delay.h> //_delay_ms()
#include "./AVRAugment_io.h"
##define LED13 B5 // Only need to change this to swap pins
#define DDR_13	SET_REG( DDR, LED13, 0xFF ) //IN=0x00, OUT=0xFF
#define ON_13	SET_REG( PORT, LED13, 0xFF ) //LOW=0x00, HIGH=0xFF
#define OFF_13	SET_REG( PORT, LED13, 0x00 ) //LOW=0x00, HIGH=0xFF
int main(void)
{
	DDR_13;
	ON_13;
	while(1)
	{
		_delay_ms(2000);
		OFF_13;
		_delay_ms(2000);
		ON_13;
	}
}


*/

