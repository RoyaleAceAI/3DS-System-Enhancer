#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"

// Overlay settings
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

static bool battery_saver = false;

// Simple animated battery bar colors
static u32 battery_color(u8 percent) {
    if(percent > 60) return C2D_Color32(0,255,0,255); // green
    else if(percent > 30) return C2D_Color32(255,255,0,255); // yellow
    else return C2D_Color32(255,0,0,255); // red
}

int main() {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init();
    C2D_Prepare(); // start 2D
    C2D_SceneBegin(GFX_TOP);
    C2D_Target* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    consoleInit(GFX_BOTTOM, NULL);

    printf("3DS System Enhancer Overlay\n");
    printf("Press START to exit overlay, SELECT to toggle battery saver.\n");

    battery_saver = read_bool_config(CONFIG_PATH, "battery_saver", false);
    set_battery_saver(battery_saver);

    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if(kDown & KEY_START) break;
        if(kDown & KEY_SELECT) {
            battery_saver = !battery_saver;
            set_battery_saver(battery_saver);
            write_bool_config(CONFIG_PATH, "battery_saver", battery_saver);
        }

        // Begin drawing top screen
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32(0,0,0,128));
        C2D_SceneBegin(top);

        // Battery bar
        u8 battery = get_battery_percent();
        float bar_width = 200 * battery / 100.0f;
        C2D_DrawRectSolid(50, 100, 0, bar_width, 30, battery_color(battery));

        // Battery outline
        C2D_DrawRectOutline(50, 100, 0, 200, 30, C2D_Color32(255,255,255,255), 2.0f);

        // Battery % text
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", battery);
        C2D_DrawTextBuf(C2D_TextBufNew(), 130, 105, 0.0f, 1.0f, 1.0f, buf, C2D_Color32(255,255,255,255));

        // Battery saver icon/text
        if(battery_saver) {
            C2D_DrawRectSolid(50, 150, 0, 50, 30, C2D_Color32(0,128,255,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55, 155, 0.0f, 0.7f, 0.7f, "Saver ON", C2D_Color32(255,255,255,255));
        } else {
            C2D_DrawRectSolid(50, 150, 0, 50, 30, C2D_Color32(128,128,128,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55, 155, 0.0f, 0.7f, 0.7f, "Saver OFF", C2D_Color32(255,255,255,255));
        }

        // Low battery pulse
        if(battery <= 20) {
            u32 t = svcGetSystemTick() / 1000000;
            u8 alpha = (t%1000 < 500) ? 255 : 128;
            C2D_DrawRectSolid(50, 50, 0, 300, 50, C2D_Color32(255,0,0,alpha));
        }

        C3D_FrameEnd(0);

        // Bottom screen text
        consoleSelect(GFX_BOTTOM);
        printf("\x1b[2J");
        printf("Battery: %d%%\n", battery);
        printf("Battery Saver: %s\n", battery_saver?"ON":"OFF");
        printf("Press START to exit overlay, SELECT to toggle battery saver.\n");
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
