#include <xc.h>
#include "common.h"
#include "uart.h"
extern unsigned int sec, blink_f;
extern unsigned char ch;
void __interrupt() isr(void) {
    static unsigned short count;
    if (TMR0IF) {
        TMR0 = TMR0 + 8;

        if (count++ == 20000) {
            count = 0;
            blink_f = !blink_f;
            if (SCREEN_FLAG == PASSWORD_SCREEN)
                --sec;
        }
        TMR0IF = 0;
    }
}