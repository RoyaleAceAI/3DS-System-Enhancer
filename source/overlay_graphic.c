#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

static bool battery_saver = false;
static u32 lastTick = 0;
static float cpu_usage = 0;
static float fps = 0;
static float overlayOffset = -SCREEN_WIDTH; // slide overlay
static u32 theme_background = C2D_Color32(0,0,0,128);

// Battery color function
static u32 battery_color(u8 percent) {
    if(percent > 60) return C2D_Color32(0,255,0,255);
    else if(percent > 30) return C2D_Color32(255,255,0,255);
    else return C2D_Color32(255,0,0,255);
}

// Fake CPU usage for demo
static void update_cpu_usage() {
    cpu_usage = (svcGetSystemTick() % 1000) / 10.0f;
}

// FPS calculation
static void update_fps() {
    u32 tick = svcGetSystemTick();
    fps = 1000000.0f / (tick - lastTick + 1);
    lastTick = tick;
}

// Draw battery bar with fill animation
static void draw_battery_bar(float x, float y, u8 percent, float fill_ratio) {
    float bar_width = 200 * percent / 100.0f * fill_ratio;
    C2D_DrawRectSolid(x, y, 0, bar_width, 30, battery_color(percent));
    C2D_DrawRectOutline(x, y, 0, 200, 30, C2D_Color32(255,255,255,255), 2.0f);
}

int main() {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init();
    C2D_Prepare();
    C2D_Target* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    consoleInit(GFX_BOTTOM, NULL);

    battery_saver = read_bool_config(CONFIG_PATH, "battery_saver", false);
    set_battery_saver(battery_saver);

    // --- Animated Intro ---
    float introProgress = 0.0f;
    const float introSpeed = 0.02f; // 2 seconds animation

    while(introProgress < 1.0f) {
        u8 battery = get_battery_percent();
        hidScanInput();
        u32 kDown = hidKeysDown();
        if(kDown & KEY_START) return 0;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, theme_background);
        C2D_SceneBegin(top);

        // Sliding title
        float titleX = -200 + 200 * introProgress;
        C2D_DrawTextBuf(C2D_TextBufNew(), titleX, 20, 0, 1.5f, 1.5f, "3DS System Enhancer", C2D_Color32(255,255,255,255));

        // Animated battery fill
        draw_battery_bar(50, 80, battery, introProgress);

        // Fade-in CPU & FPS bars
        float alpha = introProgress * 255;
        C2D_DrawRectSolid(50, 130, 0, 200 * cpu_usage / 100.0f, 20, C2D_Color32(255,128,0,alpha));
        C2D_DrawRectSolid(50, 160, 0, 200 * fps / 60.0f, 20, C2D_Color32(0,255,128,alpha));

        C3D_FrameEnd(0);

        introProgress += introSpeed;
        gspWaitForVBlank();
    }

    // --- Main overlay loop ---
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
        if(overlayOffset < 0) overlayOffset += 8.0f;

        u8 battery = get_battery_percent();

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, theme_background);
        C2D_SceneBegin(top);

        // Battery
        draw_battery_bar(50 + overlayOffset, 50, battery, 1.0f);

        // Battery % text
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", battery);
        C2D_DrawTextBuf(C2D_TextBufNew(), 140 + overlayOffset, 55, 0, 1.0f, 1.0f, buf, C2D_Color32(255,255,255,255));

        // Battery saver
        if(battery_saver) {
            C2D_DrawRectSolid(50 + overlayOffset, 100, 0, 50, 30, C2D_Color32(0,128,255,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55 + overlayOffset, 105, 0, 0.7f, 0.7f, "Saver ON", C2D_Color32(255,255,255,255));
        } else {
            C2D_DrawRectSolid(50 + overlayOffset, 100, 0, 50, 30, C2D_Color32(128,128,128,255));
            C2D_DrawTextBuf(C2D_TextBufNew(), 55 + overlayOffset, 105, 0, 0.7f, 0.7f, "Saver OFF", C2D_Color32(255,255,255,255));
        }

        // CPU & FPS bars
        C2D_DrawRectSolid(50 + overlayOffset, 150, 0, 200 * cpu_usage / 100.0f, 20, C2D_Color32(255,128,0,255));
        snprintf(buf, sizeof(buf), "CPU: %.1f%%", cpu_usage);
        C2D_DrawTextBuf(C2D_TextBufNew(), 50 + overlayOffset, 145, 0, 0.6f, 0.6f, buf, C2D_Color32(255,255,255,255));

        C2D_DrawRectSolid(50 + overlayOffset, 180, 0, 200 * fps / 60.0f, 20, C2D_Color32(0,255,128,255));
        snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
        C2D_DrawTextBuf(C2D_TextBufNew(), 50 + overlayOffset, 175, 0, 0.6f, 0.6f, buf, C2D_Color32(255,255,255,255));

        // Low battery pulse
        if(battery <= 20) {
            u32 t = svcGetSystemTick() / 1000000;
            u8 alpha = (t%1000 < 500) ? 255 : 128;
            C2D_DrawRectSolid(overlayOffset, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, C2D_Color32(255,0,0,alpha));
        }

        C3D_FrameEnd(0);

        consoleSelect(GFX_BOTTOM);
        printf("\x1b[2J");
        printf("Battery: %d%%\n", battery);
        printf("Battery Saver: %s\n", battery_saver?"ON":"OFF");
        printf("CPU: %.1f%%\nFPS: %.1f\n", cpu_usage, fps);
        printf("Press START to exit, SELECT to toggle battery saver, Y to hide overlay\n");
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
