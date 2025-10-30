// enhanced_settings.c
// Graphical "Enhanced Settings" homebrew app for 3DS System Enhancer
// Uses citro2d. Interacts with the same config JSON as overlay.

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "system_utils.h"
#include "json_parser.h"

#define CONFIG_PATH "/3ds/system_enhancer/config.json"

#define SCREEN_W 400
#define SCREEN_H 240

typedef enum {
    PAGE_MAIN = 0,
    PAGE_DISPLAY,
    PAGE_POWER,
    PAGE_PERFORMANCE,
    PAGE_ADVANCED,
    PAGE_COUNT
} Page;

// UI state
static Page currentPage = PAGE_MAIN;
static int cursor = 0;
static bool battery_saver = false;
static int brightness = 100; // 0-100 (we store in config)
static bool show_help = false;

// Helpers to read/write config keys
static void load_settings() {
    battery_saver = read_bool_config(CONFIG_PATH, "battery_saver", false);
    // brightness stored as key "brightness", as 0/100 integer
    FILE *f = fopen(CONFIG_PATH, "r");
    if(!f) return;
    char line[128];
    while(fgets(line, sizeof(line), f)) {
        char k[64], v[64];
        if(sscanf(line, "%63[^=]=%63s", k, v) == 2) {
            if(strcmp(k, "brightness") == 0) {
                brightness = atoi(v);
            }
        }
    }
    fclose(f);
}

static void save_settings() {
    // we will write both keys (simple overwrite file)
    FILE *f = fopen(CONFIG_PATH, "w");
    if(!f) return;
    fprintf(f, "battery_saver=%d\n", battery_saver?1:0);
    fprintf(f, "brightness=%d\n", brightness);
    fclose(f);
}

// draw a small label with optional highlight
static void draw_label(C2D_TextBuf textBuf, float x, float y, const char* text, bool highlight) {
    C2D_Text textObj;
    C2D_TextBufClear(textBuf);
    C2D_TextBufAdd(textBuf, text);
    textObj = C2D_TextParse(textBuf);
    float sx = highlight ? 1.05f : 1.0f;
    u32 color = highlight ? C2D_Color32(255,255,0,255) : C2D_Color32(255,255,255,255);
    C2D_DrawText(&textObj, C2D_WithColor, x, y, 0.5f, sx, sx, color);
}

// draw a slider (x,y,width) with value 0..100
static void draw_slider(float x, float y, float w, int value) {
    float filled = (w * value) / 100.0f;
    C2D_DrawRectSolid(x, y, 0, filled, 10, C2D_Color32(0,160,255,255));
    C2D_DrawRectOutline(x, y, 0, w, 10, C2D_Color32(255,255,255,255), 2.0f);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", value);
    C2D_DrawTextBuf(C2D_TextBufNew(), x+w+8, y-2, 0, 0.6f, 0.6f, buf, C2D_Color32(255,255,255,255));
}

int main(int argc, char **argv) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init();
    C2D_Prepare();
    C2D_Target* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    consoleInit(GFX_BOTTOM, NULL);

    // load persisted settings
    load_settings();
    set_battery_saver(battery_saver);
    gfxSetBrightness(GFX_TOP, brightness);
    gfxSetBrightness(GFX_BOTTOM, brightness);

    // Text buffers reused across frames
    C2D_TextBuf titleBuf = C2D_TextBufNew();
    C2D_TextBuf itemBuf = C2D_TextBufNew();
    C2D_TextBuf infoBuf = C2D_TextBufNew();

    u32 lastTick = svcGetSystemTick();
    while(aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if(kDown & KEY_START) break;
        if(kDown & KEY_L) { // quick save
            save_settings();
        }
        if(kDown & KEY_SELECT) {
            show_help = !show_help;
        }

        // Navigation: left/right change page, up/down move cursor, A toggles/activates
        if(kDown & KEY_RIGHT) {
            currentPage = (currentPage + 1) % PAGE_COUNT;
            cursor = 0;
        } else if(kDown & KEY_LEFT) {
            currentPage = (currentPage - 1 + PAGE_COUNT) % PAGE_COUNT;
            cursor = 0;
        } else if(kDown & KEY_DOWN) {
            cursor++;
        } else if(kDown & KEY_UP) {
            cursor--;
        }
        if(cursor < 0) cursor = 0;
        if(cursor > 5) cursor = 5; // safety upper limit

        // Page interactions
        if(currentPage == PAGE_MAIN) {
            if(kDown & KEY_A) {
                // A opens the first page (Display)
                currentPage = PAGE_DISPLAY;
                cursor = 0;
            }
            if(kDown & KEY_Y) {
                // Launch graphical overlay immediately
                // Attempt to launch overlay app by asking user to run it from HOME.
                // We can't start another .3dsx from here easily; instead we write a "launch" flag.
                FILE *f = fopen("/3ds/system_enhancer/launch_overlay.flag","w");
                if(f) { fprintf(f,"1\n"); fclose(f); }
            }
        } else if(currentPage == PAGE_DISPLAY) {
            if(kDown & KEY_LEFT) { brightness = (brightness - 10); if(brightness < 0) brightness = 0; gfxSetBrightness(GFX_TOP, brightness); gfxSetBrightness(GFX_BOTTOM, brightness); }
            if(kDown & KEY_RIGHT) { brightness = (brightness + 10); if(brightness > 100) brightness = 100; gfxSetBrightness(GFX_TOP, brightness); gfxSetBrightness(GFX_BOTTOM, brightness); }
            if(kDown & KEY_A) {
                // confirm save
                save_settings();
            }
        } else if(currentPage == PAGE_POWER) {
            if(kDown & KEY_A) {
                battery_saver = !battery_saver;
                set_battery_saver(battery_saver);
                save_settings();
            }
        } else if(currentPage == PAGE_PERFORMANCE) {
            // nothing toggleable yet; maybe future logging
            if(kDown & KEY_A) {
                // toggle a demo log file
                FILE *f = fopen("/3ds/system_enhancer/perf_log.flag","w");
                if(f) { fprintf(f,"1\n"); fclose(f); }
            }
        } else if(currentPage == PAGE_ADVANCED) {
            if(kDown & KEY_A) {
                // export config: copy config file to /3ds/system_enhancer/config_backup.txt
                FILE *in = fopen(CONFIG_PATH, "r");
                FILE *out = fopen("/3ds/system_enhancer/config_backup.txt", "w");
                if(in && out) {
                    char buf[256];
                    while(fgets(buf, sizeof(buf), in)) fputs(buf, out);
                    fclose(in); fclose(out);
                }
            }
        }

        // draw UI
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32(8,8,16,255));
        C2D_SceneBegin(top);

        // Title
        C2D_Text title = C2D_TextParse(C2D_TextBufAdd(titleBuf, "Enhanced Settings"));
        C2D_DrawText(&title, C2D_WithColor, 16, 8, 0, 1.2f, 1.2f, C2D_Color32(200,200,255,255));

        // Page indicator
        const char* pageNames[PAGE_COUNT] = { "Main", "Display", "Power", "Performance", "Advanced" };
        char pageLabel[64];
        snprintf(pageLabel, sizeof(pageLabel), "[%s]  (Left/Right to change)", pageNames[currentPage]);
        C2D_DrawTextBuf(itemBuf, 16, 36, 0, 0.6f, 0.6f, pageLabel, C2D_Color32(200,200,200,255));

        // Draw current page contents
        switch(currentPage) {
            case PAGE_MAIN: {
                C2D_DrawTextBuf(itemBuf, 24, 60, 0, 0.9f, 0.9f, "Overview", C2D_Color32(255,255,255,255));
                char line[128];
                snprintf(line, sizeof(line), "Battery: %d%%", get_battery_percent());
                C2D_DrawTextBuf(infoBuf, 24, 90, 0, 0.8f, 0.8f, line, C2D_Color32(255,255,255,255));
                snprintf(line, sizeof(line), "Battery Saver: %s", battery_saver ? "ON" : "OFF");
                C2D_DrawTextBuf(infoBuf, 24, 110, 0, 0.8f, 0.8f, line, C2D_Color32(255,255,255,255));
                snprintf(line, sizeof(line), "Brightness: %d%% (Left/Right to adjust on Display page)", brightness);
                C2D_DrawTextBuf(infoBuf, 24, 130, 0, 0.7f, 0.7f, line, C2D_Color32(220,220,220,255));
                C2D_DrawTextBuf(infoBuf, 24, 160, 0, 0.7f, 0.7f, "Press Y to request overlay launch, A to go to Display.", C2D_Color32(200,200,200,255));
                break;
            }

            case PAGE_DISPLAY: {
                C2D_DrawTextBuf(itemBuf, 24, 60, 0, 0.9f, 0.9f, "Display Settings", C2D_Color32(255,255,255,255));
                C2D_DrawTextBuf(infoBuf, 24, 90, 0, 0.7f, 0.7f, "Brightness (Left/Right to change, A to save)", C2D_Color32(200,200,200,255));
                draw_slider(24, 120, 220, brightness);
                break;
            }

            case PAGE_POWER: {
                C2D_DrawTextBuf(itemBuf, 24, 60, 0, 0.9f, 0.9f, "Power & Battery", C2D_Color32(255,255,255,255));
                char bs[64];
                snprintf(bs, sizeof(bs), "Battery Saver: %s (A to toggle)", battery_saver ? "ON" : "OFF");
                C2D_DrawTextBuf(infoBuf, 24, 90, 0, 0.8f, 0.8f, bs, C2D_Color32(255,255,255,255));
                // battery bar preview
                int batt = get_battery_percent();
                draw_slider(24, 120, 220, batt);
                break;
            }

            case PAGE_PERFORMANCE: {
                C2D_DrawTextBuf(itemBuf, 24, 60, 0, 0.9f, 0.9f, "Performance", C2D_Color32(255,255,255,255));
                char perf[128];
                snprintf(perf, sizeof(perf), "Free mem: %u bytes", get_free_memory());
                C2D_DrawTextBuf(infoBuf, 24, 90, 0, 0.7f, 0.7f, perf, C2D_Color32(200,200,200,255));
                u64 sd = get_sd_free();
                char sdbuf[128];
                snprintf(sdbuf, sizeof(sdbuf), "SD free: %llu bytes", sd);
                C2D_DrawTextBuf(infoBuf, 24, 110, 0, 0.7f, 0.7f, sdbuf, C2D_Color32(200,200,200,255));
                C2D_DrawTextBuf(infoBuf, 24, 140, 0, 0.7f, 0.7f, "Press A to mark perf logging flag", C2D_Color32(200,200,200,255));
                break;
            }

            case PAGE_ADVANCED: {
                C2D_DrawTextBuf(itemBuf, 24, 60, 0, 0.9f, 0.9f, "Advanced", C2D_Color32(255,255,255,255));
                C2D_DrawTextBuf(infoBuf, 24, 90, 0, 0.7f, 0.7f, "A: Backup config to config_backup.txt", C2D_Color32(200,200,200,255));
                C2D_DrawTextBuf(infoBuf, 24, 110, 0, 0.7f, 0.7f, "L: Quick save to config (writes now)", C2D_Color32(200,200,200,255));
                break;
            }
            default: break;
        }
