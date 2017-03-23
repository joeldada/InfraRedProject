/*
 * File:   uart.c
 * Author: joel.dada
 *
 * Created on March 23, 2017, 4:25 PM
 */
//pic18f2550
#include <pic18f2550.h>

#include "cirBuffer.h"
#include "uart.h"

#define SIZEBUFFER_RC1 128
#define SIZEBUFFER_TX1 128

/*********	 	PRIVATE UART FUNCTIONS	 	*********/

char dataXAvailable(Circular_Buffer *cbRC);
char sendXByte(unsigned char data, unsigned char waitToTx, Circular_Buffer *cbTX);
char sendXData(unsigned char *data, unsigned short size, unsigned char waitToTx, Circular_Buffer *cbTX);
char readXByte(unsigned char *data, Circular_Buffer *cbRC);
char readXData(unsigned char *data, unsigned short size, Circular_Buffer *cbRC);

char dataXAvailable(Circular_Buffer *cbRC) {
    return circularbNumElements(cbRC);
}

char sendXByte(unsigned char data, unsigned char waitToTx, Circular_Buffer *cbTX) {
    char ret = 0;

    while (circularbIsFull(cbTX));
    ret = circularbWrite(cbTX, (char *) &data);

    PIE1bits.TXIE = 0;
    if (TXSTAbits.TXEN == 0) TXSTAbits.TXEN = 1;
    PIE1bits.TXIE = 1;

    if (waitToTx) while (TXSTAbits.TRMT == 0 && TXSTAbits.TXEN == 1);
    return ret;
}

char sendXData(unsigned char *data, unsigned short size, unsigned char waitToTx, Circular_Buffer *cbTX) {
    char ret = 0;
    unsigned short index = 0;
    unsigned short sizegiven;

    sizegiven = (SIZEBUFFER_TX1 - circularbNumElements(cbTX)) < size ? (SIZEBUFFER_TX1 - circularbNumElements(cbTX)) : size;
    
    for (index = 0; index < sizegiven; index++) {
        circularbWrite(cbTX, (char *) data);

            PIE1bits.TXIE = 0;
            if (TXSTAbits.TXEN == 0) TXSTAbits.TXEN = 1;
            PIE1bits.TXIE = 1;
        
        data++;
    }
    for (; index < size; index++) {
        while (circularbIsFull(cbTX));
        circularbWrite(cbTX, (char *) data);

            PIE1bits.TXIE = 0;
            if (TXSTAbits.TXEN == 0) TXSTAbits.TXEN = 1;
            PIE1bits.TXIE = 1;
       
        data++;
    }

        if (waitToTx) while (TXSTAbits.TRMT == 0 && TXSTAbits.TXEN == 1);
    return ret;
}

char readXByte(unsigned char *data, Circular_Buffer *cbRC) {
    char ret = 0;
    while (circularbIsEmpty(cbRC));
    ret = circularbRead(cbRC, (char *) data);
    return ret;
}

char readXData(unsigned char *data, unsigned short size, Circular_Buffer *cbRC) {
    char ret = 0;
    unsigned short index = 0;
    unsigned short sizegiven;

    sizegiven = circularbNumElements(cbRC) < size ? circularbNumElements(cbRC) : size;
    for (index = 0; index < sizegiven; index++) {
        circularbRead(cbRC, (char *) data);
        data++;
    }
    for (; index < size; index++) {
        while (circularbIsEmpty(cbRC));
        circularbRead(cbRC, (char *) data);
        data++;
    }

    return ret;
}

/*********	 	UART1	 	*********/

Circular_Buffer cbRC1;
Circular_Buffer cbTX1;
char buff_rc1[SIZEBUFFER_RC1];
char buff_tx1[SIZEBUFFER_TX1];

void initUart(void) {

    /* UART1 CONFIGURATION */

    TX9 = 0;
    TXSTAbits.TXEN = 0;
    SYNC = 0;
    SENDB = 0;
    BRGH = 1;

    SPEN = 1;
    RX9 = 0;
    CREN = 1;
    ADDEN = 0;

    RXDTP = 0;
    TXCKP = 0;
    BRG16 = 1;
    WUE = 0;
    ABDEN = 0;

    /* DATASHEET
     * 9600 =  FOSC/(4 * (spbrg+1))
     * spbrg = FOSC/(9600*4) - 1
     * SPBRGH1 = 0;
     * SPBRG1 = 77; */
    SPBRGH = 0x04;
    SPBRG = 0xE1;

    TRISC7 = 1; // Input RX
    TRISC6 = 0; // Output TX

    /* BLUETOOTH CONFIGURATION */

    circularbInit(&cbRC1, buff_rc1, SIZEBUFFER_RC1);
    circularbInit(&cbTX1, buff_tx1, SIZEBUFFER_TX1);

    /* INTERRUPTION ENABLE */

    PIE1bits.TXIE = 1;
    RCIE = 1;
    RCIP = 1;
    TXIP = 1;

    IPEN = 1;
    GIE = 1;
    PEIE = 1;
}

char dataAvailable(void) {
    return dataXAvailable(&cbRC1);
}

char sendByte(unsigned char data, unsigned char waitToTx) {
    return sendXByte(data, waitToTx, &cbTX1);
}

char sendData(unsigned char *data, unsigned short size, unsigned char waitToTx) {
    return sendXData(data, size, waitToTx, &cbTX1);
}

char readByte(unsigned char *data) {
    return readXByte(data, &cbRC1);
}

char readData(unsigned char *data, unsigned short size) {
    return readXData(data, size, &cbRC1);
}

void handler_uart_int(void) {
    unsigned char toTransmit, readed;
    if (TXIF == 1 && PIE1bits.TXIE == 1) {
        if (!circularbIsEmpty(&cbTX1)) {
            circularbRead(&cbTX1, (char *) &toTransmit);
            TXREG = toTransmit;
        } else {
            if (TXSTAbits.TRMT) TXSTAbits.TXEN = 0;
        }
    }

    if (RCIF == 1 && RCIE == 1) {
        readed = RCREG;
        if (!circularbIsFull(&cbRC1)) {
            circularbWrite(&cbRC1, (char *) &readed);
        }
        RCIF = 0;
    }
}

