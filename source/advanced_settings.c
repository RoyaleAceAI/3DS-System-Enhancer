#include <3ds.h>
#include <stdio.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    consoleInit(GFX_BOTTOM, NULL);

    printf("3DS System Enhancer - Advanced Settings\n");
    printf("Press START to exit\n");

    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if(kDown & KEY_START) break;

        bool battery_saver = read_bool_config(CONFIG_PATH, "battery_saver", false);

        consoleSelect(GFX_TOP);
        printf("\x1b[2J");
        printf("Battery: %d%%\n", get_battery_percent());
        printf("Battery Saver: %s\n", battery_saver?"ON":"OFF");
        printf("Free Memory: %u bytes\n", get_free_memory());
        printf("SD Card Free: %llu bytes\n", get_sd_free());

        consoleSelect(GFX_BOTTOM);
        printf("\x1b[2J");
        printf("Press START to exit overlay\n");
        printf("Press SELECT to toggle battery saver\n");

        if(kDown & KEY_SELECT) {
            battery_saver = !battery_saver;
            set_battery_saver(battery_saver);
            write_bool_config(CONFIG_PATH, "battery_saver", battery_saver);
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
