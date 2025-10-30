#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Very tiny key=value parser for config files
static inline bool read_bool_config(const char* path, const char* key, bool default_val) {
    FILE *f = fopen(path, "r");
    if(!f) return default_val;
    char line[128];
    while(fgets(line, sizeof(line), f)) {
        char k[64]; char v[64];
        if(sscanf(line, "%63[^=]=%63s", k, v) == 2) {
            if(strcmp(k, key)==0) {
                fclose(f);
                return (strcmp(v,"1")==0);
            }
        }
    }
    fclose(f);
    return default_val;
}

static inline void write_bool_config(const char* path, const char* key, bool val) {
    FILE *f = fopen(path, "w");
    if(!f) return;
    fprintf(f,"%s=%d\n", key, val ? 1 : 0);
    fclose(f);
}

#endif
