#include <Arduino.h>
#include "LiquidCrystal.h"

#define EN 23
#define RS 22
#define Bit4 24
#define Bit5 25
#define Bit6 26
#define Bit7 27

#define OEN 41
#define SEL1 40
#define SEL2 39
#define RST 38

//_____________________________________
//MOTOR DE BAIXO
#define SpeedA 45       //  PL4
#define DirectionA 43   //  PL6
#define BrakeA 47       //  PL2
#define FaultA 49       //  PL0

//_____________________________________
//MOTOE DE CIMA
#define SpeedB 44       //  PL5
#define DirectionB 42   //  PL7
#define BrakeB 46       //  PL3
#define FaultB 48       //  PL1

#define TOP 300
#define TOP_UP 310
#define TOP_DOWN 280
#define BOTTOM 125
#define BOTTOM_UP 200
#define BOTTOM_DOWN 150

#define B1 A8
#define B2 A9
#define B3 A10
#define B4 A11
#define B5 A12

#define LED_DOWN A13
#define LED_UP A14
#define LED_STOP A15

#define pi 3.14159265

/*typedef boolean unsigned char;*/

unsigned char auxByte;
unsigned long ENC = 0;
boolean first = 1;
boolean flag = 1;
int MAX_ENC = 14000;
int MIN_ENC = 0;
unsigned long TAB_REF[] = {0, 0, 0};


void RESET();
void SOFT_START_DOWN(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta);
void SOFT_START_UP(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta);
unsigned long COUNT();
void REF_DOWN();
unsigned long REF_UP();
void SOFT_STOP_DOWN(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta );
void SOFT_STOP_UP(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta );

void PWM_init() {
  TCCR5A = _BV(COM5B1) | _BV(COM5C1) | _BV(WGM51);
  TCCR5B = _BV(WGM53) | _BV(WGM52) | _BV(CS50);
  ICR5 = 799 ; // resolution - define o "topo" do contador e desta forma faz variar a frequência
  OCR5B = BOTTOM; // 50%
  OCR5C = BOTTOM; // 50%s
}

LiquidCrystal LCD(RS, EN, Bit4, Bit5, Bit6, Bit7);

void setup() {
  LCD.begin(16, 2);
  LCD.print("A INICIAR..");

  DDRC = 0;
  pinMode(OEN, OUTPUT);
  pinMode(SEL1, OUTPUT);
  pinMode(SEL2, OUTPUT);
  pinMode(RST, OUTPUT);

  RESET();

  /*-------------------------------------*/
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);
  pinMode(B3, INPUT);
  pinMode(B4, INPUT);
  pinMode(B5, INPUT);

  pinMode(LED_UP, OUTPUT);
  pinMode(LED_DOWN, OUTPUT);
  pinMode(LED_STOP, OUTPUT);
  /*-------------------------------------*/

  pinMode(SpeedA, OUTPUT);
  pinMode(DirectionA, OUTPUT);
  pinMode(BrakeA, OUTPUT);
  pinMode(FaultA , INPUT);

  pinMode(SpeedB, OUTPUT);
  pinMode(DirectionB, OUTPUT);
  pinMode(BrakeB, OUTPUT);
  pinMode(FaultB, INPUT);

  digitalWrite(BrakeA, LOW);
  digitalWrite(BrakeB, LOW);

  digitalWrite(DirectionA, HIGH);
  digitalWrite(DirectionB, HIGH);

  PWM_init();

  LCD.clear();
  LCD.print("INICIADO!");
}


void loop() {

  if (digitalRead(B1) || first) {
    digitalWrite(LED_STOP, HIGH);
    while (digitalRead(B1)) {}
    while (1) {
      if (digitalRead(B2)) {
        SOFT_START_DOWN(4, TOP_DOWN, 90, 25, 15);

        while (digitalRead(B2)) {}
        SOFT_STOP_DOWN(4, TOP_DOWN, 90, 30, 5);
      }


      if (digitalRead(B3)) {
        SOFT_START_UP(4, TOP_UP, 90, 25, 15);

        while (digitalRead(B3)) {}
        SOFT_STOP_UP(4, TOP_UP, 90, 25, 15);
      }

      if (digitalRead(B5)) {
        while (digitalRead(B5)) {}
        
        if (TAB_REF[0] == 0) {
          LCD.clear();
          LCD.print("Posicao 1");
          TAB_REF[0] = COUNT();
        }
        else if (TAB_REF[1] == 0) {
          LCD.setCursor(0, 0);
          LCD.print("Posicao 2");
          TAB_REF[1] = COUNT();
        }
        else {
          LCD.setCursor(0, 0);
          LCD.print("Posicao 3");
          TAB_REF[2] = COUNT();
          _delay_ms(1000);
          LCD.clear();
          LCD.print("PRONTO !");
        }
      }

      if (digitalRead(B4)) {
        REF_DOWN();
        LCD.setCursor(0, 1);
        LCD.print("Ref. a Zero!");
        _delay_ms(1000);

        LCD.setCursor(0, 1);
        LCD.print("TOPO:          ");
        LCD.setCursor(6, 1);
        LCD.print(REF_UP());
        _delay_ms(1000);
      }

      if (digitalRead(B1))
        break;
    }
    digitalWrite(LED_STOP, LOW);
    while (digitalRead(B1));
  }


  uint8_t i = 0;
  while (!digitalRead(B1)) {
    ENC = COUNT();
    i = 1;
    while (ENC < (TAB_REF[2] - 2000)) {
      if (flag) {
        SOFT_START_UP(4, TOP_UP, 80, 25, 20);
        flag = !flag;
      }
      ENC = COUNT();
      if (ENC >= (TAB_REF[i] - 2000)) {
        SOFT_STOP_UP(4, TOP_UP, 80, 20, 15);
        flag = 1;
        i++;

        _delay_ms(10000);
      }
      _delay_us(3);
    }

    ENC = COUNT();
    i = 1;
    while (ENC > (TAB_REF[0] + 2000)) {
      if (flag) {
        SOFT_START_DOWN(4, TOP_DOWN, 80, 25, 20);
        flag = !flag;
      }
      ENC = COUNT();
      if (ENC <= (TAB_REF[i] + 2000)) {
        SOFT_STOP_DOWN(4, TOP_DOWN, 80, 25, 15);
        flag = 1;
        i--;

        _delay_ms(10000);
      }
      _delay_us(3);
    }
    _delay_us(3);
  }
}


void REF_DOWN() {
  unsigned long Last;
  unsigned long New;
  New = COUNT();
  SOFT_START_DOWN(2, 120, 70, 50, 40);
  _delay_ms(200);
  while (1) {
    int i = 0;
    while (i < 100) {
      Last = COUNT();
      _delay_ms(2);
      i++;
    }
    if (New == Last) {
      SOFT_STOP_DOWN(2, 120, 70, 50, 10);
      RESET();
      break;
    }
    New = Last;
  }
}

unsigned long REF_UP() {
  unsigned long Last;
  unsigned long New;
  New = COUNT();
  SOFT_START_UP(2, 170, 70, 50, 35);
  _delay_ms(200);
  while (1) {
    int j = 0;
    while (j < 100) {
      Last = COUNT();
      _delay_ms(2);
      j++;
    }
    if (New == Last) {
      SOFT_STOP_UP(2, 160, 70, 50, 10);
      return (New);
    }
    New = Last;
  }
}

/* paragem suave dos motores */
void SOFT_STOP_UP(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta ) {//inserir um maximo e minimo..
  Delta = (mult * 180 * Delta) / 100;
  Max2 = (mult * 180 * Max2) / 100;
  Min = (mult * 180 * Min) / 100;

  int i = 180 * mult;
  int j = (180 * mult) - Delta;
  int aux = 0;
  while (i > Min) {
    aux = (Top / 2) * sin((j * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    OCR5B = aux;

    aux = (Top / 2) * sin((i * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    if (aux < Max2)
      OCR5C = aux;
    i--;
    j--;
    _delay_us(500);
  }
  digitalWrite(BrakeA, LOW);
  digitalWrite(BrakeB, LOW);
}

void SOFT_STOP_DOWN(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta ) {

  Delta = (mult * 180 * Delta) / 100;
  Max2 = (mult * 180 * Max2) / 100;
  Min = (mult * 180 * Min) / 100;

  int i = (180 * mult);
  int j = (180 * mult) - Delta;
  int aux = 0;

  while (i > Min) {
    aux = (Top / 2) * sin((j * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    OCR5C = aux;

    aux = (Top / 2) * sin((i * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    if (aux < Max2)
      OCR5B = aux;
    j--;
    i--;
    _delay_us(500);
  }
  digitalWrite(BrakeB, LOW);
  _delay_ms(300);
  digitalWrite(BrakeA, LOW);

}


/* arranque suave dos motores */
void SOFT_START_UP(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta) {

  Max2 = (mult * 180 * Max2) / 100;
  Min = (mult * 180 * Min) / 100;
  Delta = (mult * 180 * Delta) / 100;

  digitalWrite(DirectionA, HIGH);
  digitalWrite(DirectionB, LOW);
  _delay_ms(3);
  digitalWrite(BrakeA, HIGH);
  digitalWrite(BrakeB, HIGH);
  int i = Min;
  int j = Min - Delta;
  int aux = 0;
  while (i < (180 * mult)) {
    aux = (Top / 2) * sin((j * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    OCR5B = aux;

    aux = (Top / 2) * sin((i * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    if (aux < Max2)
      OCR5C = aux;
    i++;
    j++;
    _delay_us(500);
  }
}


/*+++++++++++++++ CONDIÇÕES DE ARRANQUE
  mult=4
  Max=300
  Min=250 PWM_B=81
  Min-Delta=100 PWM_A=14
*/

void SOFT_START_DOWN(uint8_t mult, uint16_t Top, uint16_t Max2, uint16_t Min, uint16_t Delta) {

  Max2 = (mult * 180 * Max2) / 100;
  Min = (mult * 180 * Min) / 100;
  Delta = (mult * 180 * Delta) / 100;

  digitalWrite(DirectionA, LOW);
  digitalWrite(DirectionB, HIGH);
  _delay_ms(3);
  digitalWrite(BrakeA, HIGH); //LOW ACTIVE
  digitalWrite(BrakeB, HIGH);

  int i = Min;
  int j = Min - Delta;
  int aux = 0;

  while (i < (180 * mult)) {
    aux = (Top / 2) * sin((i * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    OCR5B = aux;

    aux = (Top / 2) * sin((j * pi / (180 * mult)) + (270 * pi / 180)) + (Top / 2);
    if (aux < Max2)
      OCR5C = aux;
    i++;
    j++;
    _delay_us(500);
  }
}


/*void STOP_UP() {
  for (int j = TOP_UP; j > BOTTOM_UP; j--) {
    OCR5C--;
    OCR5B--;

    _delay_ms(1);
  }
  digitalWrite(BrakeA, LOW);
  digitalWrite(BrakeB, LOW);
}

void STOP_DOWN() {
  for (int j = TOP_DOWN; j > BOTTOM_DOWN; j--) {
    OCR5C--;
    OCR5B--;
    _delay_ms(1);
  }
  digitalWrite(BrakeB, LOW);
  digitalWrite(BrakeA, LOW);
}*/


/*void START_UP() {
  OCR5B = BOTTOM_UP - 25;
  OCR5C = BOTTOM_UP;
  digitalWrite(DirectionA, HIGH);
  digitalWrite(DirectionB, LOW);
  delay(3);
  digitalWrite(BrakeA, HIGH);
  digitalWrite(BrakeB, HIGH);
  int j = BOTTOM_UP;
  while (j < TOP_UP) {
    OCR5B++;
    OCR5C++;
    j++;
    _delay_ms(2);
  }
}

void START_DOWN() {
  OCR5B = BOTTOM_DOWN;
  OCR5C = BOTTOM_DOWN - 75; //75
  digitalWrite(DirectionA, LOW);
  digitalWrite(DirectionB, HIGH);
  delay(3);
  digitalWrite(BrakeA, HIGH); //LOW ACTIVE
  digitalWrite(BrakeB, HIGH);

  int i = BOTTOM_DOWN;
  while (i < TOP_DOWN) {
    OCR5B++;
    OCR5C++;
    i++;
    _delay_ms(2);
  }
}*/

/* ler encoder */
unsigned long COUNT() {
  unsigned long auxCount = 0;
  digitalWrite(OEN, LOW);
  digitalWrite(SEL1, LOW);
  digitalWrite(SEL2, HIGH);
  delayMicroseconds(1);
  auxByte = PINC;
  auxCount = auxByte;
  auxCount <<= 8;

  digitalWrite(SEL1, HIGH);
  digitalWrite(SEL2, HIGH);
  delayMicroseconds(1);
  auxByte = PINC;
  auxCount += auxByte;
  auxCount <<= 8;

  digitalWrite(SEL1, LOW);
  digitalWrite(SEL2, LOW);
  delayMicroseconds(1);
  auxByte = PINC;
  auxCount += auxByte;
  auxCount <<= 8;

  digitalWrite(SEL1, HIGH);
  digitalWrite(SEL2, LOW);
  delayMicroseconds(1);
  auxByte = PINC;
  auxCount += auxByte;

  digitalWrite(OEN, HIGH);
  return (auxCount);
}

/* reset do encoder */
void RESET() {
  digitalWrite(RST, LOW);
  delayMicroseconds(5);
  digitalWrite(RST, HIGH);
  delayMicroseconds(1);
}



