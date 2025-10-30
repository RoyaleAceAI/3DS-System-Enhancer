3DS System Enhancer – Installation & Usage Guide

Version: October 2025
Apps included:

overlay_graphic.3dsx – graphical system overlay (battery %, CPU/FPS, battery saver, themes)

enhanced_settings.3dsx – full-featured enhanced settings homebrew app

1. Requirements

Hardware:

Nintendo 3DS / 2DS / New 3DS (firmware supported by homebrew, e.g., 11.16.0-49U)

SD card (2GB+ recommended)

Software:

Homebrew Launcher installed (e.g., via [FBI] or compatible boot method)

devkitPro with ctrulib & citro2d (latest stable release as of 2025)

Includes devkitARM

Citro2d is required for graphical rendering

A text editor (for editing config.json if needed)

Windows/Linux/macOS command line with make

2. Preparing the Project

Clone or download the project repository to your PC. Example:

git clone https://gitlab.com/yourusername/3ds-system-enhancer.git
cd 3ds-system-enhancer


Verify the folder structure:

3ds-system-enhancer/
├── Makefile
├── README.md
├── source/
│   ├── overlay_graphic.c
│   ├── enhanced_settings.c
│   ├── system_utils.h
│   └── json_parser.h


Ensure devkitPro environment variables are correctly set:

DEVKITPRO points to your devkitPro installation

DEVKITARM should exist inside DEVKITPRO

3. Compiling the Apps

Open a terminal/command prompt in the project root.

Run make:

make


This will compile:

overlay_graphic.3dsx

enhanced_settings.3dsx

If you want to clean previous builds:

make clean


After compilation, you should see .3dsx files in the project root.

4. Preparing the SD Card

Insert your 3DS SD card into your PC.

Create a folder for your homebrew apps if it doesn’t exist:

/3ds/


Copy the compiled apps to the SD card:

/3ds/overlay_graphic.3dsx
/3ds/enhanced_settings.3dsx


Create a configuration folder on the SD card:

/3ds/system_enhancer/


Optionally, create a blank config.json file inside:

{
  "battery_saver": false,
  "brightness": 100
}


The apps will automatically create/update this file if missing.

5. Running the Apps

Insert the SD card back into your 3DS.

Launch Homebrew Launcher.

From the launcher, select enhanced_settings.3dsx:

Use D-Pad to navigate pages.

Left/Right: switch pages

Up/Down: move cursor (for future interactive elements)

A: toggle/save/apply items

L: quick save settings

SELECT: toggle help overlay

START: exit to Homebrew Launcher

To run the overlay_graphic:

Launch overlay_graphic.3dsx from Homebrew Launcher

Displays battery %, CPU/FPS graphs, and battery saver toggle.

Press START to exit overlay, SELECT to toggle battery saver, Y to hide overlay.

Optional integration:

In enhanced_settings.3dsx, pressing Y creates a launch_overlay.flag file.

You can manually launch overlay_graphic.3dsx after that to have the overlay show your configured settings.

6. Config File Usage

File location: /3ds/system_enhancer/config.json

JSON keys:

{
  "battery_saver": true,
  "brightness": 80
}


battery_saver: true/false

brightness: 0–100 (applied to both top and bottom screen)

Both enhanced_settings.3dsx and overlay_graphic.3dsx read/write this file automatically.

7. Notes & Tips

Safe to use: This is homebrew only. It does not overwrite official 3DS System Settings.

Battery Saver toggle: Reduces CPU usage slightly (simulated).

Brightness: Adjusts top & bottom screen brightness in real-time.

Overlay: Can be hidden or shown dynamically.

Advanced pages: Include config backup and future performance stats.

8. Troubleshooting

App won’t launch:

Verify .3dsx files are in /3ds/

Verify Homebrew Launcher works

Make sure SD card is FAT32/exFAT formatted

Graphics missing / broken:

Ensure citro2d is installed via devkitPro

Make sure project compiled successfully with no errors

Config not saved:

Ensure /3ds/system_enhancer/config.json exists and is writable

9. Future Enhancements

Smooth overlay fade in/out

Custom icons & fonts for overlay

Import/export config UI

Optional auto-launch of overlay from enhanced_settings

✅ You’re ready! Insert SD card, launch enhanced_settings.3dsx, and enjoy your fully graphical, modernized 3DS System Enhancer
