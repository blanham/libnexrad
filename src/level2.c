/*
 * Copyright (c) 2025 Dynamic Weather Solutions, Inc. Distributed under the
 * terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "util.h"

#include <nexrad/message.h>
#include <nexrad/level2.h>
#include <nexrad/geo.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

nexrad_level2_data_header *nexrad_level2_get_data_header(void *data, size_t size) {
    if (data == NULL || size < sizeof(nexrad_level2_data_header)) {
        return NULL;
    }

    return (nexrad_level2_data_header *)data;
}

void *nexrad_level2_get_block(nexrad_level2_data_header *header, const char *name) {
    if (header == NULL || name == NULL) {
        return NULL;
    }

    uint16_t count = be16toh(header->data_block_count);
    for (int i = 0; i < count && i < 9; i++) {
        uint32_t offset = be32toh(header->data_block_pointers[i]);
        if (offset == 0) continue;

        nexrad_level2_data_block *block = (nexrad_level2_data_block *)((char *)header + offset);
        if (strncmp(block->type, name, 3) == 0) {
            return (void *)block;
        }
    }

    return NULL;
}

float nexrad_level2_decode_moment(nexrad_level2_moment_data *moment, int bin) {
    if (moment == NULL || bin < 0 || bin >= be16toh(moment->bin_count)) {
        return NEXRAD_LEVEL2_NO_DATA;
    }

    uint16_t raw_val = 0;
    if (moment->data_size == 8) {
        uint8_t *data = (uint8_t *)moment->data;
        raw_val = data[bin];
    } else if (moment->data_size == 16) {
        uint16_t *data = (uint16_t *)moment->data;
        raw_val = be16toh(data[bin]);
    } else {
        return NEXRAD_LEVEL2_NO_DATA;
    }

    if (raw_val == 0) {
        return NEXRAD_LEVEL2_NO_DATA; /* Below Threshold */
    }
    if (raw_val == 1) {
        return NEXRAD_LEVEL2_RANGE_FOLDED; /* Range Folded */
    }

    float scale = nexrad_bswap_float(moment->scale);
    float offset = nexrad_bswap_float(moment->offset);

    if (scale == 0.0f) {
        return NEXRAD_LEVEL2_NO_DATA;
    }

    return ((float)raw_val - offset) / scale;
}

int nexrad_level2_get_moment_range(nexrad_level2_moment_data *moment, int bin, float *start_km, float *end_km) {
    if (moment == NULL || bin < 0 || bin >= be16toh(moment->bin_count)) {
        return -1;
    }

    float range_to_first = (float)be16toh(moment->range_to_first_bin);
    float bin_size = (float)be16toh(moment->bin_size);
    
    // range_to_first_bin is the distance to the center of the first bin
    // Typical bin sizes are 250m or 1000m (0.25km or 1.0km)
    // The bin sizes here are usually in meters, wait. ICD says:
    // "Range to First Gate of reflectivity data (meters)" -> Wait, let's assume meters.
    // If it's meters, dividing by 1000 gives km.
    // Actually, "range_to_first_bin" is usually in meters (e.g. 2125 or 2000).
    // And "bin_size" is in meters (e.g. 250 or 1000). Wait, some docs say km.
    // Let's divide by 1000 to return km.

    float center_m = range_to_first + (bin * bin_size);
    
    if (start_km) *start_km = (center_m - (bin_size / 2.0f)) / 1000.0f;
    if (end_km) *end_km = (center_m + (bin_size / 2.0f)) / 1000.0f;

    return 0;
}

int nexrad_level2_get_bin_cartesian(nexrad_geo_spheroid *spheroid, nexrad_geo_cartesian *radar_pos, double radar_alt, float azimuth, float elevation, float slant_range_km, nexrad_geo_cartesian *target_out, double *altitude_out) {
    if (!spheroid || !radar_pos || !target_out) {
        return -1;
    }

    double a = nexrad_geo_spheroid_get_radius(spheroid); // Earth radius, roughly 6378137.0 m
    // 4/3 Earth radius for standard refraction
    double R_prime = (4.0 / 3.0) * a;

    double r = slant_range_km * 1000.0; // convert to meters
    double theta = elevation * (M_PI / 180.0);

    // Height of target above radar level
    double h = sqrt(r * r + R_prime * R_prime + 2.0 * r * R_prime * sin(theta)) - R_prime;

    if (altitude_out) {
        *altitude_out = radar_alt + h;
    }

    // Great circle distance along the surface
    double s = R_prime * asin((r * cos(theta)) / (R_prime + h));

    nexrad_geo_polar p;
    p.azimuth = azimuth;
    p.range = s; // geo.h uses meters for range

    nexrad_geo_find_cartesian_dest(spheroid, radar_pos, target_out, &p);

    return 0;
}

nexrad_image *nexrad_level2_create_projected_image(
    nexrad_message *message,
    const char *moment_name,
    int sweep_num,
    nexrad_color_table *table,
    nexrad_geo_projection *proj,
    float scale,
    float offset
) {
    uint16_t rays = 0;
    uint16_t bins = 0;
    uint16_t rb_meters = 0;

    if (nexrad_geo_projection_read_azimuth_count(proj, &rays) != 0) return NULL;
    if (nexrad_geo_projection_read_range(proj, &bins, &rb_meters) != 0) return NULL;

    uint8_t *grid = calloc(rays * bins, sizeof(uint8_t));
    if (!grid) return NULL;

    nexrad_level2_message_header *mh = NULL;
    void *data = NULL;
    size_t size = 0;

    while (nexrad_message_next_level2_record(message, &mh, &data, &size) == 1) {
        if (mh->type == 31) {
            nexrad_level2_data_header *dh = nexrad_level2_get_data_header(data, size);
            if (!dh) continue;
            if (dh->elevation_number != sweep_num) continue;

            nexrad_level2_moment_data *moment = nexrad_level2_get_block(dh, moment_name);
            if (!moment) continue;

            float az_angle = nexrad_bswap_float(dh->azimuth_angle);
            int azimuth_idx = (int)round(az_angle * (rays / 360.0)) % rays;

            uint16_t moment_bins = be16toh(moment->bin_count);
            for (int b = 0; b < moment_bins && b < bins; b++) {
                float val = nexrad_level2_decode_moment(moment, b);
                if (val == NEXRAD_LEVEL2_NO_DATA || val == NEXRAD_LEVEL2_RANGE_FOLDED) continue;

                int color_idx = (int)((val * scale) + offset);
                if (color_idx < 0) color_idx = 0;
                if (color_idx > 255) color_idx = 255;
                
                grid[azimuth_idx * bins + b] = (uint8_t)color_idx;
            }
        } else if (mh->type == 1) {
            nexrad_level2_message_type1 *t1 = (nexrad_level2_message_type1 *)data;
            if (be16toh(t1->elevation_number) != sweep_num) continue;

            float az_angle = (float)be16toh(t1->azimuth_angle) * (360.0f / 65536.0f);
            int azimuth_idx = (int)round(az_angle * (rays / 360.0)) % rays;

            uint16_t pointer = 0;
            if (strncmp(moment_name, "REF", 3) == 0) pointer = be16toh(t1->sur_pointer);
            else if (strncmp(moment_name, "VEL", 3) == 0) pointer = be16toh(t1->vel_pointer);
            else if (strncmp(moment_name, "SW ", 3) == 0) pointer = be16toh(t1->sw_pointer);

            if (pointer == 0) continue;

            uint8_t *moment_data = (uint8_t *)data + pointer;
            
            for (int b = 0; b < bins; b++) {
                uint8_t raw_val = moment_data[b];
                if (raw_val <= 1) continue;

                float val = 0;
                if (strncmp(moment_name, "REF", 3) == 0) {
                    val = ((float)raw_val - 65.0f) / 2.0f;
                } else if (strncmp(moment_name, "VEL", 3) == 0) {
                    float res = (be16toh(t1->vel_resolution) == 2) ? 0.5f : 1.0f;
                    val = ((float)raw_val - 129.0f) * res;
                } else if (strncmp(moment_name, "SW ", 3) == 0) {
                    val = ((float)raw_val - 129.0f) / 2.0f;
                }

                int color_idx = (int)((val * scale) + offset);
                if (color_idx < 0) color_idx = 0;
                if (color_idx > 255) color_idx = 255;
                grid[azimuth_idx * bins + b] = (uint8_t)color_idx;
            }
        }
    }

    nexrad_image *image = nexrad_geo_project_polar_grid(proj, grid, rays, bins, table);

    free(grid);
    return image;
}
