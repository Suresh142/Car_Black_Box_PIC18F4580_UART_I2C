#include <xc.h>
#include "common.h"
#include "i2c.h"
#include "ds1307.h"
#include "adc.h"
#include "main.h"
#include "timer0.h"
#include "matrix_keypad.h"
#include "uart.h"

void init_config(void) {
    init_matrix_keypad();
    init_clcd();
    init_adc();
    init_timer0();
    init_i2c();
    init_ds1307();
    //init_uart();
    GIE = 1;
    PEIE = 1;
    /*TO store 0th EVENT in EEPROM */
    store_event_in_eeprom();
//    write_external_eeprom(0x78, 0);
//    write_external_eeprom(0x79, 0);
//    write_external_eeprom(0x7A, 0);
//    write_external_eeprom(0x7B, 0);
}

void main(void) {
    CLEAR_DISP_SCREEN;
    init_config();

    while (1) {
        key = read_switches(STATE_CHANGE);

        if (key == MK_SW5 && flag) {
            SCREEN_FLAG = PASSWORD_SCREEN;
            flag = 0;
        }

        if (SCREEN_FLAG == DASHBOARD) {
            display_dashboard();
        } else if (SCREEN_FLAG == PASSWORD_SCREEN) {
            password_screen();
        } else if (SCREEN_FLAG == MENU_SCREEN) {
            if (MENU_FLAG == VIEW_LOG) {
                view_log();
            } else if (MENU_FLAG == DOWNLOAD_LOG) {
                download_log();
            } else if (MENU_FLAG == CLEAR_LOG) {
                clear_log();
            } else if (MENU_FLAG == SER_TIME) {
                key = read_switches(LEVEL_CHANGE);
                set_time(key);
            } else if (MENU_FLAG == CHANGE_PASSWORD) {
                change_password();
            } else
                display_menu();
        }
    }
}