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

#ifndef _NEXRAD_L2_MODEL_H
#define _NEXRAD_L2_MODEL_H

#include <stdint.h>
#include <sys/types.h>

typedef struct {
    char name[4];              /* REF, VEL, SW, ZDR, PHI, RHO, etc. */
    uint16_t bin_count;
    int32_t first_gate_m;
    uint16_t gate_width_m;
    uint8_t data_word_bits;    /* 8 or 16 */
    float scale;
    float offset;
    uint8_t *raw;
    size_t raw_len;
} nexrad_l2_moment_view;

typedef struct {
    uint16_t elevation_number;
    uint8_t cut_sector_number;
    uint8_t radial_status;
    float azimuth_deg;
    float elevation_deg;
    nexrad_l2_moment_view *moments;
    size_t moment_count;
} nexrad_l2_radial_view;

typedef struct {
    nexrad_l2_radial_view **radials;
    size_t radial_count;
    uint16_t sweep_number;
    float elevation_deg;
} nexrad_l2_sweep_view;

typedef struct {
    nexrad_l2_sweep_view **sweeps;
    size_t sweep_count;
    char icao[5];
    double lat, lon, alt;
} nexrad_l2_volume_view;

struct _nexrad_message;

nexrad_l2_volume_view *nexrad_l2_volume_parse(struct _nexrad_message *message);
void nexrad_l2_volume_destroy(nexrad_l2_volume_view *volume);

#endif /* _NEXRAD_L2_MODEL_H */
