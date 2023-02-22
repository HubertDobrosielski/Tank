
/**
 * @brief funkcja inicjująca USART, ustawiane są odpowiedznie rejestry
 */
void USART_Init(void);

/**
 * @brief funkcja wysyła pojedynczy znak do terminala
 * @param data znak któy chcemy wysłać (wpisujemy w pojedynczych '')
 */
void USART_Transmit(unsigned char data);

/**
 * @brief funkcja wysyła do terminala ciąg znaków (zapętlona funkcja USART_Tramsmit)
 * @param stringToSend ciąg znaków któe chcemy wysłać (wpisujemy w "")
 */
void USART_Transmit_String(char *stringToSend);