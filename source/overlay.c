#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    consoleInit(GFX_BOTTOM, NULL);

    printf("3DS System Enhancer Overlay\n");
    printf("Press START to exit overlay\n");

    // read battery saver from config
    bool battery_saver = read_bool_config(CONFIG_PATH, "battery_saver", false);
    set_battery_saver(battery_saver);

    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if(kDown & KEY_START) break;
        if(kDown & KEY_SELECT) { // toggle battery saver
            battery_saver = !battery_saver;
            set_battery_saver(battery_saver);
            write_bool_config(CONFIG_PATH, "battery_saver", battery_saver);
        }

        // draw overlay info
        consoleSelect(GFX_TOP);
        printf("\x1b[2J"); // clear
        printf("Battery: %d%%\n", get_battery_percent());
        printf("Battery Saver: %s (press SELECT to toggle)\n", battery_saver?"ON":"OFF");

        consoleSelect(GFX_BOTTOM);
        printf("\x1b[2J");
        printf("Press START to exit overlay\n");

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
