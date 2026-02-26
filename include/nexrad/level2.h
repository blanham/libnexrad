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

#ifndef _NEXRAD_LEVEL2_H
#define _NEXRAD_LEVEL2_H

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

#define NEXRAD_LEVEL2_VOLUME_HEADER_SIZE 24

typedef struct _nexrad_level2_volume_header {
    char     tape[9];
    char     extension[3];
    uint32_t date;
    uint32_t time;
    char     icao[4];
} nexrad_level2_volume_header;

typedef struct _nexrad_level2_message_header {
    uint16_t size;         /* Size of message in 16-bit halfwords */
    uint8_t  rda_channel;
    uint8_t  type;         /* Message type */
    uint16_t seq;          /* ID sequence number */
    uint16_t date;         /* Modified Julian Date */
    uint32_t time;         /* Generation time in milliseconds past midnight */
    uint16_t segments;     /* Number of message segments */
    uint16_t segment_num;  /* Message segment number */
} nexrad_level2_message_header;

typedef struct _nexrad_level2_record_header {
    char     icao[4];
    uint32_t time;
    uint16_t date;
    uint16_t radial_num;
    float    azimuth;
    uint16_t compress_idx;
    uint16_t radial_spacing;
} nexrad_level2_record_header;

/* Message Type 31 */

typedef struct _nexrad_level2_data_header {
    char     radar_id[4];
    uint32_t collection_time;
    uint16_t collection_date;
    uint16_t azimuth_number;
    float    azimuth_angle;
    uint8_t  compress_idx;
    uint8_t  spare;
    uint16_t radial_length;
    uint8_t  azimuth_resolution_spacing;
    uint8_t  radial_status;
    uint8_t  elevation_number;
    uint8_t  cut_sector_number;
    float    elevation_angle;
    uint8_t  radial_blanking_status;
    uint8_t  azimuth_indexing_mode;
    uint16_t data_block_count;
    uint32_t data_block_pointers[9];
} nexrad_level2_data_header;

typedef struct _nexrad_level2_data_block {
    char     name[1]; /* "R", "E", "V", "D", "S", "Z", "C", "P", "K" */
    char     type[3]; /* "VOL", "ELV", "RAD", "REF", "VEL", "SW ", "ZDR", "PHI", "RHO" */
} nexrad_level2_data_block;

typedef struct _nexrad_level2_volume_data {
    nexrad_level2_data_block block;
    uint16_t size;
    uint8_t  version_major;
    uint8_t  version_minor;
    float    lat;
    float    lon;
    int16_t  alt;
    uint16_t feed_horn_height;
    float    calibration;
    float    v_reflectivity;
    float    h_reflectivity;
    float    differential_reflectivity;
    float    initial_differential_phase;
    uint16_t vcp;
    uint16_t spare;
} nexrad_level2_volume_data;

typedef struct _nexrad_level2_elevation_data {
    nexrad_level2_data_block block;
    uint16_t size;
    uint16_t atmos;
    float    calibration;
} nexrad_level2_elevation_data;

typedef struct _nexrad_level2_radial_data {
    nexrad_level2_data_block block;
    uint16_t size;
    uint16_t bin_count;
    float    range_to_first_bin;
    float    bin_size;
} nexrad_level2_radial_data;

typedef struct _nexrad_level2_moment_data {
    nexrad_level2_data_block block;
    uint16_t size;
    uint16_t bin_count;
    float    range_to_first_bin;
    float    bin_size;
    uint16_t snr_threshold;
    uint8_t  control_flags;
    uint8_t  data_size;
    float    scale;
    float    offset;
    uint8_t  data[1];
} nexrad_level2_moment_data;

/* Message Type 1: Digital Radar Data (Legacy) */

typedef struct _nexrad_level2_message_type1 {
    uint32_t time;
    uint16_t date;
    uint16_t azimuth_number;
    float    azimuth_angle;
    uint8_t  radial_status;
    uint8_t  elevation_number;
    uint16_t cut_sector_number;
    float    elevation_angle;
    uint16_t spot_blanking_status;
    uint16_t azimuth_indexing_mode;
    uint16_t sur_pointer;  /* Reflectivity */
    uint16_t vel_pointer;  /* Velocity */
    uint16_t sw_pointer;   /* Spectrum Width */
    uint16_t vel_resolution;
    uint16_t vcp;
    uint16_t spare1[4];
    uint16_t nyquist_velocity;
    uint16_t atmos_attenuation;
    uint16_t threshold_parameter;
    uint16_t spot_blanking_bitmap;
    uint16_t spare2[32];
    /* Data follows at pointers */
} nexrad_level2_message_type1;

#pragma pack(pop)

nexrad_level2_data_header *nexrad_level2_get_data_header(void *data, size_t size);
void *nexrad_level2_get_block(nexrad_level2_data_header *header, const char *name);

#endif /* _NEXRAD_LEVEL2_H */
