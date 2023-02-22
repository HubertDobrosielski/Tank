#include <avr/interrupt.h>
#include "usart.h"
#include <util/atomic.h>
#define COMMAND_SIZE 6

/**
 * @brief funkcja ustawia timer do obsługi PWM silników
 */
void counterPwmSet();

/**
 * @brief funkcja ustawia timery do kierunkowskazów
 */
void counterTurnSignalsSet();

/**
 * @brief funkcja włącza lub wyłącza kierunkowskazy
 *
 * @param onL włączenie(true)/wyłaczenie(false) kierunkowskazu lewego
 * @param onR włączenie(true)/wyłaczenie(false) kierunkowskazu prawego
 */
void turnSignsOnOff(bool onL, bool onR);

/**
 * @brief osługa kierunkoskazów
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void turnSigns(volatile int *sl, volatile int *sr);

void counterPwmLightsSet();

/**
 * @brief funkcja ustawia początkowe wartości pinów
 */
void pinInit();

/**
 * @brief funkcja ustawia funkcje pinów
 */
void setPins();

/**
 * @brief zwiększ prędkość w kierunku do przodu (w przypadku poruszania się
do przodu) lub zmniejsz prędkość poruszania się do tyłu (w przypadku poruszania
się do tyłu)
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
*/
void FWD(volatile int *sl, volatile int *sr);

/**
 * @brief zmniejsz prędkość w kierunku do przodu (w przypadku poruszania się
do przodu) lub zwiększ prędkość poruszania się do tyłu (w przypadku poruszania
się do tyłu)
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void REW(volatile int *sl, volatile int *sr);

/**
 * @brief zwiększ skręt w lewo
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void LEFT(volatile int *sl, volatile int *sr);

/**
 * @brief zwiększ skręt w prawo
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void RIGHT(volatile int *sl, volatile int *sr);

/**
 * @brief zatrzymaj pojazd.
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void STOP(volatile int *sl, volatile int *sr);

/**
 * @brief funkcja powoduje awaryjne zatrzymanie pojazdu
 *
 * @param sl (speedLEFT) - Prędkość lewej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 * @param sr (speedRight) - Prędkość prawej gąsienicy,
 * zawiera także kierunek ("+" - do przodu, "-" - do tyłu)
 */
void emergencySTOP(volatile int *sl, volatile int *sr);

void Buzzer(volatile int *sl, volatile int *sr);

// obsługa świateł przednich oraz buzzera
void frontLights(volatile int *sl, volatile int *sr);


void stopSet();
void speedDisplay(volatile int sl, volatile int sr);