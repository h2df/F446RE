#include "mbed.h"
#include "uart.h"

void Uart::rxHandler(void)
{
    char c = getc();
    if (! rxQ.isFull()) {
        rxQ.push(c);
        if (c == '\n') {
            newlines++;
        }
    }
}

void Uart::txHandler(void)
{
    if (! txQ.isEmpty()) {
        char c = txQ.pop();
        putc(c);
    }
}

void Uart::putChar(char c)
{
    while (txQ.isFull());
    txQ.push(c);
    if (writeable()) {
        txHandler();
    }
}

void Uart::putString(char *s)
{
    int i = 0;
    while (s[i] != '\0') {
        putChar(s[i]);
        i++;
    }
}

char Uart::getChar(void)
{
    while (rxQ.isEmpty());
    char c = rxQ.pop();
    if (c == '\n') newlines--;
    return c;
}

void Uart::readLine(char *s)
{
    while(! canReadLine());
    int i = 0;
    while ((s[i]=getChar()) != '\n') {
        i++;
    }
    i++;
    s[i] = '\0';
}
