#include "mbed.h"
#include "queue.h"

class Uart : public RawSerial
{
public:
    Uart(PinName tx, PinName rx) : RawSerial(tx, rx), newlines(0) {
        attach(callback(this, &Uart::rxHandler), RxIrq);
        attach(callback(this, &Uart::txHandler), TxIrq);
    }
    void putChar(char c);
    void putString(char *s);
    bool isDataReady(void) {
        return ! rxQ.isEmpty();
    }
    char getChar(void);
    bool canReadLine(void) {
        return newlines;
    }
    void readLine(char *s);
    void writeLine(char *s) {
        putString(s);
        putChar('\n');
    }
    void rxHandler(void);
    void txHandler(void);
protected:
    Queue txQ;
    Queue rxQ;
    volatile int newlines; //volatile makes actions atomic, so ++ and -- won't be interrupted.
};