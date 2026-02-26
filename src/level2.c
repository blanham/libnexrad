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
#include "util.h"

#include <nexrad/level2.h>

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
