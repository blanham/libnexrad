#ifndef _NEXRAD_COLOR_H
#define _NEXRAD_COLOR_H

#include <stdint.h>

#define NEXRAD_COLOR_TABLE_MAGIC      "CLUT"
#define NEXRAD_COLOR_TABLE_ENTRY_SIZE 3

typedef struct _nexrad_color_table {
    char    magic[4]; /* Always "CLUT" */
    uint8_t size;     /* Number of entries in table (powers of 2) */
} nexrad_color_table;

typedef struct _nexrad_color_table_entry {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} nexrad_color_table_entry;

nexrad_color_table *nexrad_color_table_create(uint8_t size);

void nexrad_color_table_store_entry(nexrad_color_table *table,
    uint8_t index, uint8_t r, uint8_t g, uint8_t b
);

nexrad_color_table *nexrad_color_table_open(const char *path);

int nexrad_color_table_save(nexrad_color_table *table,
    const char *path,
    nexrad_color_table_entry *entries
);

void nexrad_color_table_close(nexrad_color_table *table);

#endif /* _NEXRAD_COLOR_H */