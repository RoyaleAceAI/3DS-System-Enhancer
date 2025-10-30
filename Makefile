DEVKITPRO ?= /opt/devkitpro
include $(DEVKITPRO)/3dsRules

TARGETS := overlay advanced_settings

overlay_SOURCES := source/overlay.c source/system_utils.h source/json_parser.h
overlay_TARGET := overlay

advanced_settings_SOURCES := source/advanced_settings.c source/system_utils.h source/json_parser.h
advanced_settings_TARGET := advanced_settings

all: $(TARGETS:%=%.3dsx)

clean:
	-rm -f *.elf *.3dsx *.smdh

.PHONY: all clean
