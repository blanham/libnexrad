/*
 * Copyright (c) 2013-2026 Bryce Lanham. Distributed under the
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

#include <nexrad/level2.h>
#include <nexrad/message.h>
#include <nexrad/l2_model.h>
#include "util.h"

void nexrad_l2_volume_destroy(nexrad_l2_volume_view *volume) {
    if (!volume) return;

    for (size_t i = 0; i < volume->sweep_count; i++) {
        nexrad_l2_sweep_view *sweep = volume->sweeps[i];
        if (!sweep) continue;

        for (size_t j = 0; j < sweep->radial_count; j++) {
            nexrad_l2_radial_view *radial = sweep->radials[j];
            if (!radial) continue;

            for (size_t k = 0; k < radial->moment_count; k++) {
                free(radial->moments[k].raw);
            }
            free(radial->moments);
            free(radial);
        }
        free(sweep->radials);
        free(sweep);
    }
    free(volume->sweeps);
    free(volume);
}

static nexrad_l2_sweep_view *_get_or_create_sweep(nexrad_l2_volume_view *vol, uint16_t sweep_num) {
    for (size_t i = 0; i < vol->sweep_count; i++) {
        if (vol->sweeps[i]->sweep_number == sweep_num) {
            return vol->sweeps[i];
        }
    }

    vol->sweeps = realloc(vol->sweeps, sizeof(nexrad_l2_sweep_view *) * (vol->sweep_count + 1));
    nexrad_l2_sweep_view *sweep = calloc(1, sizeof(nexrad_l2_sweep_view));
    sweep->sweep_number = sweep_num;
    vol->sweeps[vol->sweep_count++] = sweep;
    return sweep;
}

nexrad_l2_volume_view *nexrad_l2_volume_parse(nexrad_message *message) {
    if (!message || message->level != NEXRAD_LEVEL_2) return NULL;

    nexrad_l2_volume_view *vol = calloc(1, sizeof(nexrad_l2_volume_view));
    if (!vol) return NULL;

    safecpy(vol->icao, message->level2_volume_header->icao, 5, 4);

    nexrad_message_reset_level2(message);

    nexrad_level2_message_header *mh = NULL;
    void *data = NULL;
    size_t size = 0;

    while (nexrad_message_next_level2_record(message, &mh, &data, &size) == 1) {
        if (mh->type == 31) {
            nexrad_level2_data_header *dh = nexrad_level2_get_data_header(data, size);
            if (!dh) continue;

            nexrad_l2_sweep_view *sweep = _get_or_create_sweep(vol, dh->elevation_number);
            sweep->elevation_deg = nexrad_bswap_float(dh->elevation_angle);

            nexrad_l2_radial_view *radial = calloc(1, sizeof(nexrad_l2_radial_view));
            radial->elevation_number = dh->elevation_number;
            radial->cut_sector_number = dh->cut_sector_number;
            radial->radial_status = dh->radial_status;
            radial->azimuth_deg = nexrad_bswap_float(dh->azimuth_angle);
            radial->elevation_deg = nexrad_bswap_float(dh->elevation_angle);

            // Iterate through blocks
            uint16_t block_count = be16toh(dh->data_block_count);
            for (int i = 0; i < block_count && i < 9; i++) {
                uint32_t offset = be32toh(dh->data_block_pointers[i]);
                if (offset == 0 || offset > size || size - offset < 4) continue;

                nexrad_level2_data_block *block = (nexrad_level2_data_block *)((char *)dh + offset);
                if (memcmp(block->type, "D", 1) == 0) { // Moment block
                    nexrad_level2_moment_data *moment = (nexrad_level2_moment_data *)block;
                    radial->moments = realloc(radial->moments, sizeof(nexrad_l2_moment_view) * (radial->moment_count + 1));
                    nexrad_l2_moment_view *mv = &radial->moments[radial->moment_count++];
                    
                    memcpy(mv->name, block->name, 3);
                    mv->name[3] = '\0';
                    mv->bin_count = be16toh(moment->bin_count);
                    mv->first_gate_m = (int32_t)be16toh(moment->range_to_first_bin);
                    mv->gate_width_m = be16toh(moment->bin_size);
                    mv->data_word_bits = moment->data_size;
                    mv->scale = nexrad_bswap_float(moment->scale);
                    mv->offset = nexrad_bswap_float(moment->offset);
                    
                    size_t bytes_per_bin = (mv->data_word_bits == 8) ? 1 : 2;
                    mv->raw_len = mv->bin_count * bytes_per_bin;
                    mv->raw = malloc(mv->raw_len);
                    // Moment data starts after header
                    memcpy(mv->raw, (uint8_t *)moment + sizeof(nexrad_level2_moment_data), mv->raw_len);
                } else if (memcmp(block->type, "V", 1) == 0 && memcmp(block->name, "VOL", 3) == 0) {
                    nexrad_level2_volume_data *rv = (nexrad_level2_volume_data *)block;
                    vol->lat = nexrad_bswap_float(rv->lat);
                    vol->lon = nexrad_bswap_float(rv->lon);
                    vol->alt = (double)be16toh(rv->alt);
                }
            }

            sweep->radials = realloc(sweep->radials, sizeof(nexrad_l2_radial_view *) * (sweep->radial_count + 1));
            sweep->radials[sweep->radial_count++] = radial;
        }
    }

    return vol;
}
