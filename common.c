#include <xc.h>
#include "common.h"
#include "i2c.h"
#include "ds1307.h"
#include "adc.h"
#include "main.h"
#include "timer0.h"
#include "matrix_keypad.h"
#include "uart.h"

extern unsigned char key;
extern unsigned int MENU_FLAG, SCREEN_FLAG, M_index;
unsigned char write_eeprom = 1, option_save = 0;
unsigned char EF = 0, EI = 0;

void display_dashboard() {
    clcd_print(" TIME     EV  SP", LINE1(0));
    get_time();
    speed = (read_adc(CHANNEL4) / 10.23);

    /* Speed Controll using Gears*/
    if (g_index == GR && speed >= 20) {
        speed = 20;
    } else if (g_index == GN || g_index == ON || g_index == C_) {
        speed = 0;
    } else if (g_index == G1 && speed >= 30) {
        speed = 30;
    } else if (g_index == G2 && speed >= 40) {
        speed = 40;
    } else if (g_index == G3 && speed >= 60) {
        speed = 60;
    } else if (g_index == G4 && speed >= 80) {
        speed = 80;
    } else if (g_index == G5 && speed >= 99) {
        speed = 99;
    }
    if (write_eeprom) {
        write_eeprom = 0;
        store_event_in_eeprom();
    }
    /* Gear system */
    if (key == MK_SW1) {
        g_index = C_;
        speed = 0;
        write_eeprom = 1;
        EF = 0;
    } else if (key == MK_SW2 && g_index <= 6 || key == MK_SW2 && g_index == C_) {
        if (g_index == C_)
            g_index = GN;
        else
            g_index++;
        write_eeprom = 1;
        EF = 0;
    } else if (key == MK_SW3 && g_index > 1 || key == MK_SW3 && g_index == C_) {
        if (g_index == C_)
            g_index = GN;
        else
            g_index--;
        write_eeprom = 1;
        EF = 0;
    }
    if (!EF) {
        clcd_print(gear[g_index], LINE2(10));
        clcd_putch((speed / 10) + '0', LINE2(14));
        clcd_putch((speed % 10) + '0', LINE2(15));
    } else {
        clcd_print(gear[EI], LINE2(10));
        clcd_putch('0', LINE2(14));
        clcd_putch('0', LINE2(15));
    }
}

/* FUNCTION DEFFINITION  PASSWORD SCREEN */
void password_screen() {
    /* NO. OF Attempts */
    if (chance > 0) {
        /* USER ENTER PASSWORD*/
        if (index < 4) {
            if (once) {
                once = 0;
                CLEAR_DISP_SCREEN;
                clcd_print(" ENTER PASSWORD ", LINE1(0));
                LCD_CURSOR_LINE2;
                LCD_CURSOR_BLINK;
                pass[0] = read_external_eeprom(0x78);
                pass[1] = read_external_eeprom(0x79);
                pass[2] = read_external_eeprom(0x7A);
                pass[3] = read_external_eeprom(0x7B);
                key = 0;
                sec = 60;
            }

            if (key == MK_SW5) {
                arr[index] = 0;
                LCD_CURSOR_BLINK;
                SHIFT_CURSOR_RIGHT;
                clcd_putch('*', LINE2(index + 4));
                if (arr[index] == pass[index]) {
                    ++count;
                }
                ++index;
                sec = 60;
            } else if (key == MK_SW6) {
                arr[index] = 1;
                LCD_CURSOR_BLINK;
                SHIFT_CURSOR_RIGHT;
                clcd_putch('*', LINE2(index + 4));
                if (arr[index] == pass[index]) {
                    ++count;
                }
                ++index;
                sec = 60;
            } else {
                if (sec < 55) {
                    DISP_ON_AND_CURSOR_OFF;
                    CLEAR_DISP_SCREEN;
                    SCREEN_FLAG = DASHBOARD;
                    index = 0;
                    count = 0;
                    sec = 60;
                    chance = 3;
                    once = 1;
                    flag = 1;
                }
            }
        } else {
            /* if PASSWORD Match or Not*/
            if (index == count) {
                DISP_ON_AND_CURSOR_OFF;
                CLEAR_DISP_SCREEN;
                clcd_print("WEL-COME TO ", LINE1(0));
                clcd_print("MENU SCREEN", LINE2(5));
                for (unsigned long y = 0xFFFFA; --y;);
                CLEAR_DISP_SCREEN;
                index = 0;
                count = 0;
                sec = 60;
                chance = 3;
                once = 1;
                /* Update To SCREEN FLAG TO MENU SCREEN */
                SCREEN_FLAG = MENU_SCREEN;
            } else {
                chance--;
                DISP_ON_AND_CURSOR_OFF;
                CLEAR_DISP_SCREEN;
                clcd_print(" WRONG PASSWORD ", LINE1(0));
                clcd_print(" ATTEMPTS ", LINE2(0));
                if (chance > 0) {
                    clcd_putch('0' + chance, LINE2(10));
                } else {
                    clcd_print("OVER", LINE2(10));
                }
                for (unsigned long y = 0xFFFFA; --y;);
                CLEAR_DISP_SCREEN;
                index = 0;
                count = 0;
                sec = 60;
                once = 1;
            }
        }
    } else {
        DISP_ON_AND_CURSOR_OFF;
        clcd_print("  USER BLOCKED  ", LINE1(0));
        clcd_print(" WAIT :    SEC", LINE2(0));
        clcd_putch('0' + sec / 10, LINE2(8));
        clcd_putch('0' + sec % 10, LINE2(9));
        if (!sec) {
            index = 0;
            count = 0;
            sec = 60;
            flag = 1;
            chance = 3;
            once = 1;
        }
    }
}



unsigned char print[12], NEW_KEY;
static unsigned int f = 0x00;
unsigned int pk = 0, Event = 0;
unsigned long delay = 0;
static unsigned int no_of_event = 0, store_add = 0x00;

void view_log() {
    clcd_print("#   TIME   EV SP", LINE1(0));
    key = read_switches(LEVEL_CHANGE);
    if (key == MK_SW5 || key == MK_SW6) {
        delay++;
        NEW_KEY = key;
    }
    if (delay < 500 && key == ALL_RELEASED) {
        if (NEW_KEY == MK_SW5 && Event <= no_of_event - 1) {
            if (Event == no_of_event - 1) {
                Event = 0;
                f = 0x00;
            } else {
                ++Event;
                f = f + 11;
            }

        } else if (NEW_KEY == MK_SW6 && Event >= 0) {
            if (Event == 0) {
                Event = no_of_event - 1;
                f = 0x0B * (no_of_event - 1);
            } else {
                --Event;
                f = f - 11;
            }
        }
        delay = 0;
        NEW_KEY = 0;
    }
    if (no_of_event > 0) {
        print[0] = read_external_eeprom(0x00 + f);
        print[1] = read_external_eeprom(0x01 + f);
        print[2] = read_external_eeprom(0x02 + f);
        print[3] = read_external_eeprom(0x03 + f);
        print[4] = read_external_eeprom(0x04 + f);
        print[5] = read_external_eeprom(0x05 + f);
        print[6] = read_external_eeprom(0x06 + f);
        print[7] = read_external_eeprom(0x07 + f);
        print[8] = ' ';
        print[9] = read_external_eeprom(0x08 + f);
        print[10] = read_external_eeprom(0X09 + f);
        print[11] = '\0';
        pk = read_external_eeprom(0x0A + f);

        clcd_putch(Event + '0', LINE2(0));
        clcd_print(print, LINE2(2));
        clcd_putch((pk / 10) + '0', LINE2(14));
        clcd_putch((pk % 10) + '0', LINE2(15));
    } else {
        clcd_print("NO EVENT's", LINE2(3));
    }
    if (delay >= 500) {
        CLEAR_DISP_SCREEN;
        SCREEN_FLAG = DASHBOARD;
        delay = 0;
        flag = 1;
        NEW_KEY = 0;
        MENU_FLAG = 0;
        f = 0x00;
    }
}


unsigned char ch;
unsigned int address;

void download_log() {
    init_uart();
    address = 0x00;
    clcd_print("DOWNLOADING...", LINE1(0));
    for (unsigned long y = 0xFFFFA; --y;);
    puts("#  TIME EV SP\n\r");
    puts("\n\r");
    if (no_of_event > 0) {
        for (int i = 0; i < no_of_event; i++) {
            print[0] = read_external_eeprom(0x00 + address);
            print[1] = read_external_eeprom(0x01 + address);
            print[2] = read_external_eeprom(0x02 + address);
            print[3] = read_external_eeprom(0x03 + address);
            print[4] = read_external_eeprom(0x04 + address);
            print[5] = read_external_eeprom(0x05 + address);
            print[6] = read_external_eeprom(0x06 + address);
            print[7] = read_external_eeprom(0x07 + address);
            print[8] = ' ';
            print[9] = read_external_eeprom(0x08 + address);
            print[10] = read_external_eeprom(0X09 + address);
            print[11] = '\0';
            pk = read_external_eeprom(0x0A + address);

            putch(i + '0');
            puts("  ");
            puts(print);
            putch(' ');
            putch((pk / 10) + '0');
            putch((pk % 10) + '0');
            puts("\n\r");
            address = address + 11;
        }
        clcd_print("DOWNLOAD COMPLETED", LINE1(0));
        for (unsigned long y = 0xFFFFA; --y;);
    } else {
        puts("DOWNLOAD LOG IS EMPTY\n\r");
    }
    SCREEN_FLAG = DASHBOARD;
    CLEAR_DISP_SCREEN;
    flag = 1;
    NEW_KEY = 0;
    MENU_FLAG = 0;
    address = 0x00;
}

void clear_log() {
    no_of_event = 0;
    MENU_FLAG = 0;
    clcd_print("CLEAR LOG", LINE1(3));
    clcd_print("SUCCESSFULLY", LINE2(3));
    for (unsigned long int y = 0xFFFFA; --y;);
    SCREEN_FLAG = DASHBOARD;
    EI = 9;
    EF = 1;
    flag = 1;
    store_add = 0x00;
    no_of_event = 0;
    option_save = 1;
    store_event_in_eeprom();
    CLEAR_DISP_SCREEN;
}



unsigned short int BLINK, FILED = 0, blink_f = 0;
unsigned char change_Time[9] = "00:00:00", clock_set[4], min = 0, hour = 0, second = 0;

void set_time(unsigned char key) {
    if (key == MK_SW5 || key == MK_SW6) {
        delay++;
        NEW_KEY = key;
    }
    if (delay < 500 && key == ALL_RELEASED) {
        if (NEW_KEY == MK_SW6) {
            if (FILED == 1) {
                ++second;
                if (second == 60) {
                    second = 0;
                }
                change_Time[6] = second / 10 + '0';
                change_Time[7] = second % 10 + '0';
            }
            if (FILED == 2) {
                ++min;
                if (min == 60) {
                    min = 0;
                }
                change_Time[3] = min / 10 + '0';
                change_Time[4] = min % 10 + '0';
            }
            if (FILED == 3) {
                ++hour;
                if (hour == 24) {
                    hour = 0;
                }
                change_Time[0] = hour / 10 + '0';
                change_Time[1] = hour % 10 + '0';
            }
        }
        if (NEW_KEY == MK_SW5 && FILED < 3) {
            FILED++;
        } else if (NEW_KEY == MK_SW5 && FILED == 3) {
            FILED = 1;
        }
        NEW_KEY = 0;
        delay = 0;
    }
    if (blink_f) {
        wait = 0;
        BLINK = FILED;
    } else {
        BLINK = 0;
    }
    if (BLINK == 1) {
        clcd_print("  ", LINE2(10));
    } else if (BLINK == 2) {
        clcd_print("  ", LINE2(7));
    } else if (BLINK == 3) {
        clcd_print("  ", LINE2(4));
    } else {
        clcd_print(change_Time, LINE2(4));
    }
    clcd_print("HH:MM:SS", LINE1(4));
    if (delay > 500) {
        if (NEW_KEY == MK_SW6) {
            CLEAR_DISP_SCREEN;
            MENU_FLAG = 0;
            NEW_KEY = 0;
            BLINK = 0;
            FILED = 0;
            flag = 1;
            delay = 0;
        }
        if (NEW_KEY == MK_SW5) {
            clock_set[0] = read_ds1307(HOUR_ADDR);
            clock_set[1] = read_ds1307(MIN_ADDR);
            clock_set[2] = read_ds1307(SEC_ADDR);
            write_ds1307(HOUR_ADDR, ((clock_set[0] & 0x00) | ((hour / 10) << 4) | (hour % 10)));
            write_ds1307(MIN_ADDR, (((clock_set[1]) & 0x00) | (((min / 10) << 4) | (min % 10))));
            write_ds1307(SEC_ADDR, ((clock_set[2] & 0x00) | ((second / 10) << 4) | (second % 10)));
            CLEAR_DISP_SCREEN;
            clcd_print("TIME  CHANGED", LINE1(2));
            clcd_print("SUCCESSFULLY", LINE2(2));
            for (unsigned long int y = 0xFFFFA; --y;);
            min = 0;
            hour = 0;
            second = 0;
            change_Time[9] = "00:00:00";
            CLEAR_DISP_SCREEN;
            SCREEN_FLAG = DASHBOARD;
            EI = 10;
            EF = 1;
            option_save = 1;
            store_event_in_eeprom();
            MENU_FLAG = 0;
            NEW_KEY = 0;
            BLINK = 0;
            FILED = 0;
            flag = 1;
            delay = 0;
        }

    }
}




unsigned int password = 0, dummy[4];
extern unsigned int MENU_FLAG;

void change_password() {
    if (index < 4) {
        if (once) {
            once = 0;
            CLEAR_DISP_SCREEN;
            clcd_print(" ENTER NEW PSW ", LINE1(0));
            LCD_CURSOR_LINE2;
            LCD_CURSOR_BLINK;
            key = 0;
        }

        if (key == MK_SW5) {
            arr[index] = 0;
            clcd_putch('*', LINE2(index + 3));
            if (password) {
                if (arr[index] == dummy[index]) {
                    ++count;
                }
            } else {
                dummy[index] = arr[index];
            }
            ++index;
        } else if (key == MK_SW6) {
            arr[index] = 1;
            clcd_putch('*', LINE2(index + 3));
            if (password) {
                if (arr[index] == dummy[index]) {
                    ++count;
                }
            } else {
                dummy[index] = arr[index];
            }
            ++index;
        }
    } else {
        /* if PASSWORD Match or Not*/
        if (index == 4 && !password) {
            CLEAR_DISP_SCREEN;
            clcd_print(" ENTER CNF PSW ", LINE1(0));
            LCD_CURSOR_LINE2;
            LCD_CURSOR_BLINK;
            index = 0;
            count = 0;
            password = 1;
        } else if (index == 4 && index == count) {
            DISP_ON_AND_CURSOR_OFF;
            CLEAR_DISP_SCREEN;
            clcd_print("PSW CHANGED ", LINE1(3));
            clcd_print("SUCCESSFULLY ", LINE1(3));
            for (unsigned long y = 0xFFFFA; --y;);
            CLEAR_DISP_SCREEN;
            EI = 11;
            EF = 1;
            option_save = 1;
            store_event_in_eeprom();
            index = 0;
            count = 0;
            once = 1;
            flag = 1;
            password = 0;
            write_external_eeprom(0x78, arr[0]);
            write_external_eeprom(0x79, arr[1]);
            write_external_eeprom(0x7A, arr[2]);
            write_external_eeprom(0x7B, arr[3]);
            SCREEN_FLAG = DASHBOARD;
            MENU_FLAG = 0;
        } else {
            DISP_ON_AND_CURSOR_OFF;
            CLEAR_DISP_SCREEN;
            clcd_print("PSW DOES NOT", LINE1(3));
            clcd_print("MATCH", LINE2(5));
            for (unsigned long y = 0xFFFFA; --y;);
            CLEAR_DISP_SCREEN;
            index = 0;
            count = 0;
            once = 1;
            password = 0;
        }
    }
}



/* MENU FLAG */
unsigned int M_index = 0, ARROW = L1;

void display_menu() {
    key = read_switches(LEVEL_CHANGE);
    if (key == MK_SW5 || key == MK_SW6) {
        delay++;
        NEW_KEY = key;
    }
    if (delay < 500 && key == ALL_RELEASED) {
        if (M_index < 3 && NEW_KEY == MK_SW5) {
            (ARROW) ? (M_index++) : (ARROW = L2);
        }
        if (M_index > 0 && NEW_KEY == MK_SW6) {
            (!ARROW) ? (M_index--) : (ARROW = L1);
        } else if (NEW_KEY == MK_SW5 && !ARROW) {
            ARROW = L2;
        } else if (NEW_KEY == MK_SW6 && ARROW) {
            ARROW = L1;
        }
        delay = 0;
        NEW_KEY = 0;
    }

    if (ARROW == L1) {
        clcd_print("->", LINE1(0));
        clcd_print("  ", LINE2(0));
    } else {
        clcd_print("  ", LINE1(0));
        clcd_print("->", LINE2(0));
    }
    clcd_print(diplay[M_index], LINE1(3));
    clcd_print(diplay[M_index + 1], LINE2(3));

    /* LONG Press Operation */
    if (delay >= 500) {
        if (NEW_KEY == MK_SW6) {
            CLEAR_DISP_SCREEN;
            SCREEN_FLAG = DASHBOARD;
            delay = 0;
            flag = 1;
            M_index = 0;
            ARROW = L1;
            NEW_KEY = 0;
        } else if (NEW_KEY == MK_SW5) {
            CLEAR_DISP_SCREEN;
            MENU_FLAG = ARROW ? (M_index + 4) : (M_index + 3);
            delay = 0;
            M_index = 0;
            ARROW = L1;
            NEW_KEY = 0;
        }
    }
}




/* OVER WRITE FUNCTION */
static int add = 0x00;

void over_write(unsigned int i) {
    unsigned char ch;

    for (unsigned short int j = 0; j < 11; j++) {
        ch = read_external_eeprom(i);
        write_external_eeprom(add, ch);
        add++;
        i++;
    }
}

/* STORE EEPROM FUCTION DEFFINITION */
void store_event_in_eeprom(void) {

    if (++no_of_event > 9) {
        /* OVER WRITE  */
        /* 0x0B = 11   0X62 = 98 */
        for (unsigned short int i = 0x0B; i < 0x62; i += 0x0B) {
            /* Function call  */
            over_write(i);
        }
        /* 0x58 = 88 and make 'add' as zero */
        store_add = 0x58;
        add = 0x00;
        no_of_event = no_of_event - 1;
    }
    if (option_save == 0) {
        /* TIME STORING */
        for (unsigned short int i = 0; i < 8; i++) {
            write_external_eeprom(store_add, time[i]);
            ++store_add;
        }

        /* EVENT STORING */
        for (unsigned short int i = 0; i < 2; i++) {
            write_external_eeprom(store_add, gear[g_index][i]);
            ++store_add;
        }

        /* SPEED STORING */
        write_external_eeprom(store_add, speed);
        ++store_add;
    } else {
        option_save = 0;
        /* TIME STORING */
        for (unsigned short int i = 0; i < 8; i++) {
            write_external_eeprom(store_add, time[i]);
            ++store_add;
        }

        /* EVENT STORING */
        for (unsigned short int i = 0; i < 2; i++) {
            write_external_eeprom(store_add, gear[EI][i]);
            ++store_add;
        }
        EI = 0;
        /* SPEED STORING */
        write_external_eeprom(store_add, 0);
        ++store_add;
    }
}



