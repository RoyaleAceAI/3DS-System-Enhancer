#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <3ds.h>
#include <stdio.h>

// Returns battery percentage 0-100
static inline u8 get_battery_percent() {
    return ((u32)powerGetBatteryLevel()) * 10; // simple conversion
}

// Enable or disable battery saving mode
static inline void set_battery_saver(bool enable) {
    if(enable) {
        gfxSetBrightness(GFX_TOP, 80);    // dim top screen
        gfxSetBrightness(GFX_BOTTOM, 80); // dim bottom screen
    } else {
        gfxSetBrightness(GFX_TOP, 100);
        gfxSetBrightness(GFX_BOTTOM, 100);
    }
}

// Returns free memory in bytes
static inline u32 get_free_memory() {
    u32 free_mem;
    svcGetInfo(&free_mem, 0x10003, CUR_PROCESS_HANDLE, 0); // CUR_PROCESS info
    return free_mem;
}

// Return SD card free space in bytes
static inline u64 get_sd_free() {
    u64 free, total;
    FS_Archive sdmc;
    FSUSER_OpenArchive(&sdmc, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    FSUSER_GetFreeSpace(&sdmc, &free, &total);
    FSUSER_CloseArchive(&sdmc);
    return free;
}

#endif
