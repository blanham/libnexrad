#ifndef NEXRAD_STATION_H
#define NEXRAD_STATION_H

#include <stddef.h>

/**
 * @struct nexrad_station_info
 * @brief Represents a NEXRAD station's metadata.
 */
typedef struct {
    const char *icao;      /**< 4-character ICAO identifier (e.g., "KTLX") */
    const char *name;      /**< Station name or city */
    const char *state;     /**< State or region */
    double latitude;       /**< Latitude in decimal degrees */
    double longitude;      /**< Longitude in decimal degrees */
    double altitude;       /**< Altitude in meters MSL */
} nexrad_station_info;

/**
 * @brief Looks up a NEXRAD station by its 4-character ICAO identifier.
 * 
 * @param icao The ICAO identifier (case-insensitive).
 * @return A pointer to the station info, or NULL if not found.
 */
const nexrad_station_info *nexrad_station_lookup(const char *icao);

/**
 * @brief Retrieves the complete list of NEXRAD stations.
 * 
 * @param count Pointer to an integer where the number of stations will be stored.
 * @return A pointer to the array of station info structures.
 */
const nexrad_station_info *nexrad_station_get_all(size_t *count);

#endif /* NEXRAD_STATION_H */
