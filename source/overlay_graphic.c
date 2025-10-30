#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"

// Overlay settings
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

static bool battery_saver = false;
static u32 lastTick = 0;
static float cpu_usage = 0;
static float fps = 0;
static float overlayOffset = -SCREEN_WIDTH; // sliding animation
static u32 theme_battery = C2D_Color32(0,255,0,255);
static u32 theme_background = C2D_Color32(0,0,0,128);

// Utility function: battery color
static u32 battery_color(u8 percent) {
    if(percent > 60) return C2D_Color32(0,255,0,255); // green
    else if(percent > 30) return C2D_Color32(255,255,0,255); // yellow
    else return C2D_Color32(255,0,0,255); // red
}

// Fake CPU usage for demo purposes
static void update_cpu_usage() {
    // simple random-ish simulation
    cpu_usage = (svcGetSystemTick() % 1000) / 10.0f;
}

// FPS calculation
static void update_fps() {
    u32 tick = svcGetSystemTick();
    fps = 1000000.0f / (tick - lastTick + 1);
    lastTick = tick;
}

int main() {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init();
    C2D_Prepare();
    C2D_SceneBegin(GFX_TOP);
    C2D_Target* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    consoleInit(GFX_BOTTOM, NULL);

    printf("3DS System Enhancer Overlay\nPress START to exit overlay, SELECT to toggle battery saver.\n");

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

        update_cpu_usage();
        update_fps();

        // Slide overlay in/out
        if(overlayOffset < 0) overlayOffset += 8.0f; // slide in
        if(kDown & KEY_Y) overlayOffset -= 8.0f; // press Y to hide

        // Begin drawing top screen
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, theme_background);
        C2D_SceneBegin(top);

        u8 battery = get_battery_percent();

        // Draw battery bar
        float bar_width = 200 * battery / 100.0f;
        C2D_DrawRectSolid(50 + overlayOffset, 50, 0, bar_width, 30, battery_color(battery));
        C2D_DrawRectOutline(50 + overlayOffset, 50, 0, 200, 30, C2D_Color32(255,255,255,255), 2.0f);

        // Battery % text
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", battery);
        C2D_DrawTextBuf(C2D_TextBufNew(), 140 + overlayOffset, 55, 0, 1.0f, 1.0f, buf, C2D_Color32(255,255,255,255));

        // Battery saver icon/text
        if(battery_saver) {
            C2D_DrawRectSolid(50 + overlayOffset, 100, 0, 50, 30, C2D_Color32(0,128,255,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55 + overlayOffset, 105, 0, 0.7f, 0.7f, "Saver ON", C2D_Color32(255,255,255,255));
        } else {
            C2D_DrawRectSolid(50 + overlayOffset, 100, 0, 50, 30, C2D_Color32(128,128,128,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55 + overlayOffset, 105, 0, 0.7f, 0.7f, "Saver OFF", C2D_Color32(255,255,255,255));
        }

        // CPU usage graph
        float cpu_bar = 200 * (cpu_usage / 100.0f);
        C2D_DrawRectSolid(50 + overlayOffset, 150, 0, cpu_bar, 20, C2D_Color32(255,128,0,255));
        C2D_DrawRectOutline(50 + overlayOffset, 150, 0, 200, 20, C2D_Color32(255,255,255,255), 2.0f);
        snprintf(buf, sizeof(buf), "CPU: %.1f%%", cpu_usage);
        C2D_DrawTextBuf(C2D_TextBufNew(), 50 + overlayOffset, 145, 0, 0.6f, 0.6f, buf, C2D_Color32(255,255,255,255));

        // FPS graph
        float fps_bar = 200 * (fps / 60.0f);
        C2D_DrawRectSolid(50 + overlayOffset, 180, 0, fps_bar, 20, C2D_Color32(0,255,128,255));
        C2D_DrawRectOutline(50 + overlayOffset, 180, 0, 200, 20, C2D_Color32(255,255,255,255), 2.0f);
        snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
        C2D_DrawTextBuf(C2D_TextBufNew(), 50 + overlayOffset, 175, 0, 0.6f, 0.6f, buf, C2D_Color32(255,255,255,255));

        // Low battery pulse
        if(battery <= 20) {
            u32 t = svcGetSystemTick() / 1000000;
            u8 alpha = (t%1000 < 500) ? 255 : 128;
            C2D_DrawRectSolid(overlayOffset, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, C2D_Color32(255,0,0,alpha));
        }

        C3D_FrameEnd(0);

        // Bottom screen text
        consoleSelect(GFX_BOTTOM);
        printf("\x1b[2J");
        printf("Battery: %d%%\n", battery);
        printf("Battery Saver: %s\n", battery_saver?"ON":"OFF");
        printf("CPU: %.1f%%\nFPS: %.1f\n", cpu_usage, fps);
        printf("Press START to exit overlay, SELECT to toggle battery saver, Y to hide overlay\n");
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
