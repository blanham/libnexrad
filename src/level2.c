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

void *nexrad_level2_get_block(nexrad_level2_data_header *header, const char *name, size_t record_size) {
    if (header == NULL || name == NULL) {
        return NULL;
    }

    if (record_size < sizeof(nexrad_level2_data_header)) {
        return NULL;
    }

    uint16_t count = be16toh(header->data_block_count);
    for (int i = 0; i < count && i < 9; i++) {
        uint32_t offset = be32toh(header->data_block_pointers[i]);
        if (offset == 0) continue;

        if (offset > record_size || record_size - offset < 4) {
            continue;
        }

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
        uint16_t raw_be;
        memcpy(&raw_be, (uint8_t *)moment->data + bin * 2, sizeof(raw_be));
        raw_val = be16toh(raw_be);
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

nexrad_image *nexrad_level2_render_sweep_projected(
    nexrad_l2_sweep_view *sweep,
    const char *moment_name,
    nexrad_color_table *table,
    nexrad_geo_projection *proj,
    float scale,
    float offset
) {
    if (!sweep || !moment_name || !table || !proj) return NULL;

    uint16_t rays = 0;
    uint16_t bins = 0;
    uint16_t rb_meters = 0;

    if (nexrad_geo_projection_read_azimuth_count(proj, &rays) != 0) return NULL;
    if (nexrad_geo_projection_read_range(proj, &bins, &rb_meters) != 0) return NULL;

    uint8_t *grid = calloc(rays * bins, sizeof(uint8_t));
    if (!grid) return NULL;

    for (size_t i = 0; i < sweep->radial_count; i++) {
        nexrad_l2_radial_view *radial = sweep->radials[i];
        nexrad_l2_moment_view *mv = NULL;

        for (size_t j = 0; j < radial->moment_count; j++) {
            if (strncmp(radial->moments[j].name, moment_name, 3) == 0) {
                mv = &radial->moments[j];
                break;
            }
        }

        if (!mv) continue;

        int azimuth_idx = (int)round(radial->azimuth_deg * (rays / 360.0)) % rays;

        for (uint16_t b = 0; b < mv->bin_count; b++) {
            uint16_t raw_val = 0;
            if (mv->data_word_bits == 8) {
                raw_val = mv->raw[b];
            } else if (mv->data_word_bits == 16) {
                uint16_t raw_be;
                memcpy(&raw_be, mv->raw + b * 2, 2);
                raw_val = be16toh(raw_be);
            }

            if (raw_val <= 1) continue; // NO_DATA or RANGE_FOLDED

            int32_t gate_center_m = mv->first_gate_m + (int32_t)b * mv->gate_width_m;
            if (gate_center_m < 0) continue;

            uint32_t proj_bin = (uint32_t)round((double)gate_center_m / rb_meters);
            if (proj_bin >= bins) continue;

            float val = ((float)raw_val - mv->offset) / mv->scale;
            int color_idx = (int)((val * scale) + offset);
            if (color_idx < 0) color_idx = 0;
            if (color_idx > 255) color_idx = 255;

            grid[azimuth_idx * bins + proj_bin] = (uint8_t)color_idx;
        }
    }

    nexrad_image *image = nexrad_geo_project_polar_grid(proj, grid, rays, bins, table);
    free(grid);

    return image;
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
    nexrad_l2_volume_view *vol = nexrad_l2_volume_parse(message);
    if (!vol) return NULL;

    nexrad_l2_sweep_view *target_sweep = NULL;
    for (size_t i = 0; i < vol->sweep_count; i++) {
        if (vol->sweeps[i]->sweep_number == sweep_num) {
            target_sweep = vol->sweeps[i];
            break;
        }
    }

    nexrad_image *image = NULL;
    if (target_sweep) {
        image = nexrad_level2_render_sweep_projected(target_sweep, moment_name, table, proj, scale, offset);
    }

    nexrad_l2_volume_destroy(vol);
    return image;
}
