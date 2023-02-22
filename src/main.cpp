#include "avr/delay.h"
#include "funkcje.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <stdlib.h> // ABS
#include <util/atomic.h>

#define START_COM 'Z'
#define STOP_COM 'K'

#define COMMAND_SIZE 6

volatile int speedLeft = 0;  // Prędkość lewej gąsienicy, zawiera także kierunek ("+" - do przodu, "-" - do tyłu) --volatile by móc urzywać tej zmiennej w przerwaniu
volatile int speedRight = 0; // Prędkość prawej gąsienicy, zawiera także kierunek ("+" - do przodu, "-" - do tyłu)

bool stop = false;

ISR(USART0_RX_vect) // funkcja obsługująca odbiór danych wraz z przerwaniem
{
  int txData = 0;                                // zmienna przechowuje dane z terminala
  static char command[COMMAND_SIZE] = "";        // tablica "przyszłej" komendy --zapamiętać od przerwania do przerwania
  static char currentCommand[COMMAND_SIZE] = ""; // tablica "przyszłej" komendy
  static int iCom = 0;                           // indeks tablicy command --static aby zapamiętać wartoć nawet po zakończeniu przerwania

  txData = UDR0; // pobieramy dane z rejestru i zapisujemy je w zmiennej txData; UDR0 -> resetr przechowujący dane z terminala

  // ZJLK

  if (txData == START_COM)                                 // jeśli wpiszemy 'Z' "początek komendy".
    iCom = 0;                                              // ustawiamy indeks komendy na pierwsze miejsce
  if ((command[0] == START_COM) || (txData == START_COM))  // jeśli 'Z' jest na początku komendy lub jeśli jest wpisane "w tym momencie"
    command[iCom++] = txData;                              // wpisujemy wartość kotka do kolejnych miejsc tablicy komendy
  if (((command[0] == START_COM) && (txData == STOP_COM))) // jeśli komenda jest kompletna (ma początek i koniec)
  {
    USART_Transmit_String("\nWpisana komenda: "); // wyświetlamy w terminalu jaką komende wpisaliśmy (bez znaku początku i końca)
    USART_Transmit_String(command);               // wypisujemy znaki
    USART_Transmit('\n');
    for (int i = 0; i < COMMAND_SIZE; i++)
    {
      currentCommand[i] = command[i]; // po zakończonym rozkodywaniu komendy
      command[i] = NULL;              // możemy ją usunąć aby nam nie przeszkadzała w przyszłości
    }                                 // jak nie usuniemy to warunek pozwalający wejść do tego bloku będzie aktywny
  }

  // OBSŁUGA KOMEND
  if (currentCommand[0] == START_COM)
  {
    switch (currentCommand[1])
    {
    case 'J':
      switch (currentCommand[2]) // tu patrzymy co jest na trzecim miejscu w komendzie
      {
      case 'F':
        FWD(&speedLeft, &speedRight);
        USART_Transmit_String("Predkosc zostala zwiekszona\n");
        speedDisplay(speedLeft, speedRight);
        USART_Transmit_String("\n");
        break;
      case 'R':
        REW(&speedLeft, &speedRight);
        USART_Transmit_String("Predkosc zostala zmniejszona\n");
        speedDisplay(speedLeft, speedRight);
        USART_Transmit_String("\n");
        break;
      default:
        USART_Transmit_String("Zly argument, do wyboru jest F i R\n\n");
        break;
      }
      break;
    case 'T':
      switch (currentCommand[2]) // tu patrzymy co jest na trzecim miejscu w komendzie (argument 1)
      {
      case 'L':
        LEFT(&speedLeft, &speedRight);
        USART_Transmit_String("Skrecamy w lewo\n");
        speedDisplay(speedLeft, speedRight);
        USART_Transmit_String("\n");
        break;
      case 'R':
        RIGHT(&speedLeft, &speedRight);
        USART_Transmit_String("Skrecamy w prawo\n");
        speedDisplay(speedLeft, speedRight);
        USART_Transmit_String("\n");
        break;
      default:
        USART_Transmit_String("Zly argument, do wyboru jest L i R\n");
        speedDisplay(speedLeft, speedRight);
        USART_Transmit_String("\n");
        break;
      }
      break;
    case 'S':
      STOP(&speedLeft, &speedRight);
      USART_Transmit_String("Pojazd zostal zatrzymany do kontroli\n");
      speedDisplay(speedLeft, speedRight);
      USART_Transmit_String("\n");
      break;
    default:
      USART_Transmit_String("Przykro mi, nie ma takiej komendy\n\n");
      break;
    }
    for (int i = 0; i < COMMAND_SIZE; i++) // zerujemy tablice aby switche wykonały się tylko raz
      currentCommand[i] = NULL;
  }
}

// int speedLeft = 100;   // Prędkość lewej gąsienicy, zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
// int speedRight = -100; // Prędkość prawej gąsienicy, zawiera także kierunek ("+" - do przodu, "-" - do tyłu)

int main(void)
{
  USART_Init();            // zainicjowanie komunikacji USART
  setPins();               // ustawienie pinów na wejścia/wyjścia
  pinInit();               // zainicjowanie ustawień wyjść układu
  counterPwmSet();         // zainicjowanie pracy silników (licznik PWM)
  counterTurnSignalsSet(); // zainicjowanie timera do kierunkowskazów i buzzera
  stopSet();               // zainicjowanie timera do świateł tylnych i buzzera
  counterPwmLightsSet();
  sei();

  while (1)
  {
    frontLights(&speedLeft, &speedRight);
    Buzzer(&speedLeft, &speedRight);
    turnSigns(&speedLeft, &speedRight); // obsługa kierunkowskazów

    if ((speedLeft != 0) && (speedRight != 0)) // jak stoi to awaryjny stop nie działa
    {
      if ((!(PINJ & (1 << PJ0))) || (!(PINJ & (1 << PJ1))) || (!(PINC & (1 << PC1))))
      {
        if (!stop)
        {
          emergencySTOP(&speedLeft, &speedRight);
          stop = true;
        }
      }
      else
        stop = false;
    }

    // Ustawianie kierunku obrotu gąsienicy prawej
    if (speedRight >= 0)
      PORTE |= (1 << PE4);
    else
      PORTE &= ~(1 << PE4);

    // Ustawianie kierunku obrotu gąsienicy lewej
    if (speedLeft >= 0)
      PORTE |= (1 << PE5);
    else
      PORTE &= ~(1 << PE5);

    OCR2B = abs(speedLeft);  // ustawienie prędkośći obrotu gąsienicy lewej
    OCR2A = abs(speedRight); // ustawienie prędkośći obrotu gąsienicy prawej
  }
  return 0;
}
