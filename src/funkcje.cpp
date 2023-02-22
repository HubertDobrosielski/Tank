#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h> // ABS
#include "funkcje.h"
#include <stdio.h>

// KIERUNKOWSKAZY
#define TURN_SIGN_PRES_OFF 0x08 // Ustawienia rejestru TCCRnB dla wyłączonego prescalera (kierunkowskazy)
#define TURN_SIGN_PRES_ON 0x0C  // Ustawienia rejestru TCCRnB dla włączonego prescalera (kierunkowskazy)
#define TURN_SIGN_OCR_VAL 31250 // wartość do której liczy licznik (62500/(1/0,5) = 31250) usyskany czas 1/2 s

// STOP
#define STOP_PRES_OFF 0x08 // Ustawienia rejestru TCCRnB dla wyłączonego prescalera (stop)
#define STOP_PRES_ON 0x0D  // Ustawienia rejestru TCCRnB dla włączonego prescalera (stop)
#define STOP_OCR_VAL 31250 // wartość do której liczy licznik

// FUNKCJE ZMIENIAJĄCE RUCH POJAZDU
#define SPEED_INTERVAL 50                    // interwał zmiany prędkości
#define SPEED_MAX 255                        // mkasymalna prędość
#define SPEED_MIN -255                       // minimalna prędkość (maks na wstecznym)
#define ZAB_MAX (SPEED_MAX - SPEED_INTERVAL) // wartośc maksymalna dopusczalna
#define ZAB_MIN (SPEED_MIN + SPEED_INTERVAL) // wartośc minimalna dopusczalna

volatile bool turnLeft = false;  // zmienna przechowująca informacje o tym czy lewy kierunkowskaz powinien być aktywny
volatile bool turnRight = false; // zmienna przechowująca informacje o tym czy prawy kierunkowskaz powinien być aktywny

volatile bool buzzer = false;     // zmienna przechowująca informacje o tym czy buzzer powinien być aktywny
volatile bool rearLights = false; // zmienna przechowująca informacje o tym czy tylne światła powinny być aktywne

// przerwanie od timera
SIGNAL(TIMER1_COMPA_vect) // kierunkowskazy
{
  if (turnLeft)
    PORTG ^= (1 << PG5);
  if (turnRight)
    PORTB ^= (1 << PB6);
  if (buzzer)
    PORTB ^= (1 << PB5);
}

// przerwanie od timera
SIGNAL(TIMER3_COMPA_vect) // buzzer i stop
{
  PORTB &= ~(1 << PB5);    // wyłączenie buzzera
  PORTE &= ~(1 << PE3);    // wyłączenie świateł tylnych
  TCCR3B |= STOP_PRES_OFF; // wyłączenie prescalera
  TCNT3 = 0;
}

void frontLights(volatile int *sl, volatile int *sr)
{
  if ((*sl > 0) && (*sr > 0))
  {
    //   PORTH |= (1 << PH3); // włączenie świateł przednich
    // else
    //   PORTH &= ~(1 << PH3);

    int srednia = ((*sl + *sr) / 2);
    OCR4A = srednia;
  }
  else
    OCR4A = 0;
}
void Buzzer(volatile int *sl, volatile int *sr)
{
  if ((*sl < 0) && (*sr < 0))
  {
    buzzer = true;
    TCCR1B |= TURN_SIGN_PRES_ON;
  }
  else
    buzzer = false;
}
void counterPwmSet()
{
  // konfiguracja działania pinów z licznikiem (w jaki spoóśb będziemy sterować pinem)
  TCCR2A = (1 << COM2A1) | (1 << COM2B1);

  // ustawienie trybu licznika
  TCCR2A |= (1 << WGM21) | (1 << WGM20);

  // ustawienie prescalera
  TCCR2B |= (1 << CS22);

  // -
  // TIMSK2 = (1 << OCIE2A);

  // wypełnienie PWM (przy tych wartościach natąpi zmiana stanu wyjścia)
  OCR2A = 0; // kanał A -> EN LEFT
  OCR2B = 0; // kanał B -> EN RIGHT
}
void counterPwmLightsSet()
{
  TCCR4A = (1 << COM4A1) | (0 << COM4A0);

  TCCR4A |= (1 << WGM42) | (1 << WGM40);

  TCCR4B |= (0 << CS42) | (0 << CS41) | (1 << CS40);

  // max 65 536
  OCR4A = 0;
}

void counterTurnSignalsSet()
{
  TCCR1A = 0;
  TCCR1B = TURN_SIGN_PRES_OFF;
  TIMSK1 = (1 << OCIE1A);
  OCR1A = TURN_SIGN_OCR_VAL;
}
void stopSet()
{
  TCCR3A = 0;
  TCCR3B = STOP_PRES_OFF;
  TIMSK3 = (1 << OCIE3A);
  OCR3A = STOP_OCR_VAL;
}
void turnSignsOnOff(bool onL, bool onR)
{
  turnRight = onR; // obsługa kierunkowskazów w timerze
  turnLeft = onL;

  if (onL || onR)
    TCCR1B |= TURN_SIGN_PRES_ON;
  else
    TCCR1B |= TURN_SIGN_PRES_OFF;

  if (!onR)
    PORTB &= ~(1 << PB6);
  if (!onL)
    PORTG &= ~(1 << PG5);
}

void turnSigns(volatile int *sl, volatile int *sr)
{
  if (*sl > *sr)
  {
    if ((abs(*sl) - abs(*sr)) >= 0)
      turnSignsOnOff(false, true);
    else
      turnSignsOnOff(true, false);
  }

  else if (*sl < *sr)
  {
    if ((abs(*sr) - abs(*sl)) >= 0)
      turnSignsOnOff(true, false);
    else
      turnSignsOnOff(false, true);
  }
  else
    turnSignsOnOff(false, false);
}
void pinInit()
{
  PORTE |= (1 << PE4);  // kierunek lewej gąsieniecy <- 1 - przód (może być i 0)
  PORTE |= (1 << PE5);  // kierunek prawej gąsieniecy <- 1 - przód (może być i 0)
  PORTH &= ~(1 << PH3); // światła przednie          <- WYŁĄCZONE
  PORTE &= ~(1 << PE3); // światła tylne             <- WYŁĄCZONE
  PORTG &= ~(1 << PG5); // kierunkowskaz lewy        <- WYŁĄCZONY
  PORTB &= ~(1 << PB6); // kierunkowskaz prawy       <- WYŁĄCZONY
  PORTB &= ~(1 << PB5); // Buzzer                    <- WYŁĄCZONY
}
void setPins()
{
  DDRB |= (1 << PB4);  // EN LEFT - power left       <- WYJŚCIE
  DDRH |= (1 << PH6);  // EN RIGHT - power right     <- WYJŚCIE
  DDRE |= (1 << PE4);  // PH LEFT - direction left   <- WYJŚCIE
  DDRE |= (1 << PE5);  // PH RIGHT - direction right <- WYJŚCIE
  DDRH |= (1 << PH3);  // LIGHTS FRONT               <- WYJŚCIE
  DDRE |= (1 << PE3);  // LIGHTS REAR                <- WYJŚCIE
  DDRG |= (1 << PG5);  // kierunkowskaz lewy         <- WYJŚCIE
  DDRB |= (1 << PB6);  // kierunkowskaz prawy        <- WYJŚCIE
  DDRB |= (1 << PB5);  // BUZZER                     <- WYJŚCIE
  DDRJ &= ~(1 << PJ0); // Lewy Czujnik               <- WEJŚCIE (bo to czujnik)
  DDRJ &= ~(1 << PJ1); // Prawy Czujnik              <- WEJŚCIE (bo to czujnik)
  DDRC &= ~(1 << PC1); // Czujnik tylny
}

void FWD(volatile int *sl, volatile int *sr)
{
  // 255-50=205 | 240
  if ((!(*sl > ZAB_MAX)) && (!(*sr > ZAB_MAX))) // zabezpieczenie przed zbyt dużą wartością
  {
    *sl += SPEED_INTERVAL;
    *sr += SPEED_INTERVAL;
  }
}
void REW(volatile int *sl, volatile int *sr)
{
  // -255+50=-205
  if ((!(*sl < ZAB_MIN)) && (!(*sr < ZAB_MIN))) // zabezpieczenie przed zbyt małą wartością
  {
    *sl -= SPEED_INTERVAL;
    *sr -= SPEED_INTERVAL;
  }
}
void LEFT(volatile int *sl, volatile int *sr)
{
  if ((!(*sl > ZAB_MAX)) && (!(*sr > ZAB_MAX))) // zabezpieczenie przed zbyt dużą wartością
    *sr += SPEED_INTERVAL / 2;
  if ((!(*sl < ZAB_MIN)) && (!(*sr < ZAB_MIN))) // zabezpieczenie przed zbyt małą wartością
    *sl -= SPEED_INTERVAL / 2;
}
void RIGHT(volatile int *sl, volatile int *sr)
{
  if ((!(*sl < ZAB_MIN)) && (!(*sr < ZAB_MIN))) // zabezpieczenie przed zbyt małą wartością
    *sr -= SPEED_INTERVAL / 2;
  if ((!(*sl > ZAB_MAX)) && (!(*sr > ZAB_MAX))) // zabezpieczenie przed zbyt dużą wartością
    *sl += SPEED_INTERVAL / 2;
}
void STOP(volatile int *sl, volatile int *sr)
{
  *sl = 0;
  *sr = 0;
  PORTE |= (1 << PE3);    // włączenie świateł tylnych
  TCCR3B |= STOP_PRES_ON; // włączenie licznika
  TIFR3 |= (1 << OCF3A);
}

void emergencySTOP(volatile int *sl, volatile int *sr)
{
  STOP(sl, sr);
  PORTB |= (1 << PB5); // włączenie buzzera
  USART_Transmit_String("Awaryjne zatrzymanie!!!\n");
}
void speedDisplay(volatile int sl, volatile int sr)
{
  char napis[100];
  sprintf(napis, "Predkosc L=%d  P=%d\n", sl, sr);
  USART_Transmit_String(napis);
}

