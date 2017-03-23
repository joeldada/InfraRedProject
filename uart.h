/* 
 * File:   uart.h
 * Author: joel.dada
 *
 * Created on March 23, 2017, 12:04 PM
 */

#ifndef UARTINTERRUPTION
#define UARTINTERRUPTION

#ifdef	__cplusplus
extern "C" {
#endif


void initUart(void);
char dataAvailable(void);
char sendByte(unsigned char data, unsigned char waitToTx);
char sendData(unsigned char *data, unsigned short size, unsigned char waitToTx);
char readByte(unsigned char *data);
char readData(unsigned char *data, unsigned short size);
void handler_uart_int(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

