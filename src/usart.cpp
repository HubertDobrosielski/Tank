#include <util/atomic.h>
#include <string.h>

typedef unsigned int uint;

void USART_Init(void)
{
  UBRR0H = 0;                                           // Set baud rate
  UBRR0L = 16;                                          // ustawianie prędkości transmisji (tabelka 22-12 strona 226)
  UCSR0A = (1 << U2X0);                                 // Enable receiver and transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // TX-wysyła, RX-odbiera, RXCIE-przerwanie odbioru
  UCSR0C = (0 << USBS0) | (3 << UCSZ00);                // Set frame format: 8data, 2stop bit
}

void USART_Transmit(unsigned char data)
{
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = data;
}

void USART_Transmit_String(char *stringToSend)
{
  uint len = strlen(stringToSend);   // pobieramy długość ciągu
  for (uint i = 0; i < len; i++)     // zapętlamy tyle razy ile znajdziemy znaków w ciągu
    USART_Transmit(stringToSend[i]); // wysyłamy każdy znak pojedynczo
}

