#include "mbed.h"
#include "uart.h"

Uart uart(USBTX, USBRX);

#define LCD_DATA 1
#define LCD_INSTRUCTION 0

int strToNum(char *s);
void buttonInit();
void sendClear();
void lcdInit(void);
void lcdCommand(unsigned char command);
void lcdClear(void);
void lcdPutChar(unsigned char c);
void lcdPutString(char *s);
void setButton();
void resetButton();
static void lcdSetRS(int mode); //mode is either LCD_DATA or LCD_INSTRUCTION
static void lcdPulseEN(void);
static void lcdInit8Bit(unsigned char command); //the first few commands of initialisation, are still in pseudo 8bit data mode

BusOut lcdData(PA_9, PA_8, PB_10, PB_4);
DigitalOut lcdEN(PC_7), lcdRS(PB_6);
InterruptIn buttonIn(PC_13);
DigitalOut greenLed(PA_5);
DigitalOut blueLed(PA_7);

volatile bool buttonPressed = false;
volatile bool clearSent = false;
volatile bool timeRead = false;

//UltraSonic
PwmOut usTrig(PB_3);
InterruptIn usEcho(PB_5);
volatile bool sonicOn = false;
Timer usTimer;
volatile int usTime = 0;
void stopSonic();
void initSonic();
void usEchoStart();
void usEchoEnd();

//Servo
PwmOut servo(PB_0);
void servoInit();
void changeServo(int degree);

//Stepper
BusOut stepper(PC_0, PC_1, PC_2, PC_3);
bool stepperOn = false;
int stepperData = 0;
bool stepperClock = true;
int stepperDelay = 4;
Ticker stepperTicker;
void stepperOnOff();
void tick();
void changeStepperDelay(int delay);

int main(void)
{
    lcdInit();
    servoInit();
    buttonInit();
    while (1) {
        if (uart.canReadLine()) {
            char s[80];
            uart.readLine(s);
            s[strlen(s)-1] = '\0';
            switch (s[0]) {
                case 'M':
                    lcdClear();
                    s[strlen(s)] = 0;
                    lcdPutString(&s[2]);
                    break;
                case 'G':
                    greenLed = !greenLed.read();
                    break;
                case 'B':
                    blueLed = !blueLed.read();
                    break;
                case 'U':
                    if (usTrig) stopSonic();
                    else initSonic();
                    break;
                case 'A':
                    changeServo(strToNum(&s[2]));
                    break;
                case 'S':
                    stepperOnOff();
                    break;
                case 'T':
                    stepperClock = !stepperClock;
                    break;
                case 'D':
                    int delay = strToNum(&s[2]);
                    changeStepperDelay(delay);
                    break;
            }
        }

        //Ultralsonic
        if (usTrig) {
            char s [80];
            sprintf(s, "U %d", usTime);
            uart.writeLine(s);
        }
        
        if (buttonPressed && !clearSent) {
            uart.writeLine("B");
            clearSent = true;
        }
    }
}

void changeStepperDelay(int delay)
{
    if (delay > 20) delay = 20;
    else if (delay < 4) delay = 4;
    stepperDelay = delay;
    stepperOnOff();
    stepperOnOff();
}

void stepperOnOff()
{
    if (stepperOn) {
        stepperOn = false;
        stepperData = 0;
        stepperTicker.detach();
        stepper.write(0);
    } else {
        stepperOn = true;
        stepperData = 1;
        stepperTicker.attach_us(tick, stepperDelay * 1000);
    }
}

void tick()
{
    stepper.write(stepperData);
    if (stepperClock) stepperData <<= 1;
    else stepperData >>= 1;
    if (stepperData > (1 << 3)) stepperData = 1;
    else if (stepperData < 1) stepperData = (1 << 3);
}

void buttonInit(){
    buttonIn.fall(&setButton);
    buttonIn.rise(&resetButton);
}

void setButton(){
    buttonPressed = true;
}

void resetButton(){
    clearSent = false;
    buttonPressed = false;
}

void servoInit(){
    servo.period_us(20000);
    servo.pulsewidth_us(1000);
}

void changeServo(int degree)
{
    if (degree > 90) degree = 90;
    else if (degree < 0) degree = 0;
    servo.pulsewidth_us(degree/90.0 * 1000 + 1000);
}

void initSonic()
{
    usTrig.period_us(50000);
    usTrig.pulsewidth_us(10);
    usEcho.rise(&usEchoStart);
    usEcho.fall(&usEchoEnd);
}

void stopSonic()
{
    usTrig = 0;
    usTime = 0;
}

void usEchoStart(void)
{
    usTimer.reset();
    usTimer.start();
}

void usEchoEnd(void)
{
    usTimer.stop();
    usTime = usTimer.read_us();
}

static void lcdSetRS(int mode)
{
    lcdRS.write(mode);
}

static void lcdPulseEN(void)
{
    lcdEN.write(1);
    wait_us(1);      // enable pulse must be >450ns
    lcdEN.write(0);
    wait_us(1);
}

static void lcdInit8Bit(unsigned char command)
{
    lcdSetRS(LCD_INSTRUCTION);
    lcdData.write(command>>4); //bottom 4 bits
    lcdPulseEN();
    wait_us(37);             //let it work on the data
}

void lcdInit(void)
{
    lcdEN.write(0); //GPIO_WriteBit(GPIOC, LCD_EN, Bit_RESET);
    wait_us(15000);  //delay for >15msec second after power on

    lcdInit8Bit(0x30);   //we are in "8bit" mode
    wait_us(4100);          //4.1msec delay
    lcdInit8Bit(0x30);   //- but the bottom 4 bits are ignored
    wait_us(100);           //100usec delay
    lcdInit8Bit(0x30);
    lcdInit8Bit(0x20);
    lcdCommand(0x28);      //we are now in 4bit mode, dual line
    lcdCommand(0x08);      //display off
    lcdCommand(0x01);      //display clear
    wait_us(2000);          //needs a 2msec delay !!
    lcdCommand(0x06);      //cursor increments
    lcdCommand(0x0f);      //display on, cursor(blinks) on
}

void lcdCommand(unsigned char command)
{
    lcdSetRS(LCD_INSTRUCTION);
    lcdData.write(command>>4);
    lcdPulseEN();              //this can't be too slow or it will time out
    lcdData.write(command & 0x0f);
    lcdPulseEN();
    wait_us(37);             //let it work on the data
}

void lcdClear(void)
{
    lcdCommand(0x01);
    wait_us(2000);
}

void lcdPutChar(unsigned char c)
{
    lcdSetRS(LCD_DATA);
    lcdData.write(c>>4);
    lcdPulseEN();              //this can't be too slow or it will time out
    lcdData.write(c & 0x0f);
    lcdPulseEN();
    wait_us(37);             //let it work on the data
}

void lcdPutString(char *s)
{
    int i = 0;
    while (s[i] != '\0') {
        lcdPutChar(s[i]);
        i++;
    }
}

int strToNum(char *s)
{
    int n;
    sscanf(s, "%d", &n);
    return n;
}
