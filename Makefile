DEVKITPRO ?= /opt/devkitpro
include $(DEVKITPRO)/3dsRules

TARGETS := overlay_graphic enhanced_settings

overlay_graphic_SOURCES := source/overlay_graphic.c source/system_utils.h source/json_parser.h
overlay_graphic_TARGET := overlay_graphic

enhanced_settings_SOURCES := source/enhanced_settings.c source/system_utils.h source/json_parser.h
enhanced_settings_TARGET := enhanced_settings

all: $(TARGETS:%=%.3dsx)

clean:
	-rm -f *.elf *.3dsx *.smdh

.PHONY: all clean
