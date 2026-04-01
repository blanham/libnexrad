#include "nexrad/station.h"
#include <string.h>
#include <stdlib.h>

extern const nexrad_station_info nexrad_stations[];
extern const size_t nexrad_station_count;

static int compare_icao(const void *a, const void *b) {
    const char *key = (const char *)a;
    const nexrad_station_info *station = (const nexrad_station_info *)b;
    return strcasecmp(key, station->icao);
}

const nexrad_station_info *nexrad_station_lookup(const char *icao) {
    if (icao == NULL) {
        return NULL;
    }
    return bsearch(icao, nexrad_stations, nexrad_station_count, sizeof(nexrad_station_info), compare_icao);
}

const nexrad_station_info *nexrad_station_get_all(size_t *count) {
    if (count != NULL) {
        *count = nexrad_station_count;
    }
    return nexrad_stations;
}
