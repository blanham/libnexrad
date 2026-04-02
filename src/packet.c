/*
 * Copyright (c) 2016 Dynamic Weather Solutions, Inc. Distributed under the
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
#include <math.h>
#include <string.h>
#include "util.h"

#include <nexrad/packet.h>
#include <nexrad/geo.h>
#include <nexrad/feature.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum nexrad_packet_type nexrad_packet_get_type(nexrad_packet *packet) {
    if (packet == NULL) return 0;

    return be16toh(packet->type);
}

int nexrad_packet_find_text_data(nexrad_packet *packet, int *i, int *j, int *color, char **data, size_t *textlen) {
    nexrad_text_packet *text;

    if (packet == NULL) {
        return -1;
    }

    text = (nexrad_text_packet *)packet;

    if (i)
        *i = (int16_t)be16toh(text->i);

    if (j)
        *j = (int16_t)be16toh(text->j);

    if (color)
        *color = be16toh(text->color);

    if (data && textlen) {
        *data    = (char *)text + sizeof(nexrad_text_packet);
        *textlen = be16toh(text->header.size) - sizeof(nexrad_text_packet) + sizeof(nexrad_packet_header);
    }

    return 0;
}

int nexrad_packet_read_text_data(nexrad_packet *packet, int *i, int *j, int *color, char *data, size_t *textlen, size_t destlen) {
    char *text;

    if (nexrad_packet_find_text_data(packet, i, j, color, &text, textlen) < 0) {
        return -1;
    }

    if (safecpy(data, text, destlen, *textlen) < 0) {
        goto error_safecpy;
    }

    return 0;

error_safecpy:
    return -1;
}

int nexrad_packet_read_cell_data(nexrad_packet *packet, int *i, int *j, char *id, size_t destlen) {
    nexrad_cell_packet *cell;

    if (packet == NULL) {
        return -1;
    }

    cell = (nexrad_cell_packet *)packet;

    if (i)
        *i = (int16_t)be16toh(cell->i);

    if (j)
        *j = (int16_t)be16toh(cell->j);

    if (id) {
        safecpy(id, cell->id, destlen, sizeof(cell->id));
    }

    return 0;
}

int nexrad_packet_read_hail_data(nexrad_packet *packet, int *i, int *j, int *probability, int *probability_severe, int *max_size) {
    nexrad_hail_packet *hail;

    if (packet == NULL || nexrad_packet_get_type(packet) != NEXRAD_PACKET_HAIL) {
        return -1;
    }

    hail = (nexrad_hail_packet *)packet;

    if (i)
        *i = (int16_t)be16toh(hail->i);

    if (j)
        *j = (int16_t)be16toh(hail->j);

    if (probability)
        *probability = (int16_t)be16toh(hail->probability);

    if (probability_severe)
        *probability_severe = (int16_t)be16toh(hail->probability_severe);

    if (max_size)
        *max_size = be16toh(hail->max_size);

    return 0;
}

int nexrad_packet_read_vector_data(nexrad_packet *packet, int *magnitude, nexrad_vector *vector) {
    nexrad_vector_packet *data;

    if (packet == NULL) {
        return -1;
    }

    data = (nexrad_vector_packet *)packet;

    if (magnitude)
        *magnitude = be16toh(data->magnitude);

    if (vector) {
        vector->i1_start = (int16_t)be16toh(data->i1_start);
        vector->j1_start = (int16_t)be16toh(data->j1_start);
        vector->i1_end   = (int16_t)be16toh(data->i1_end);
        vector->j1_end   = (int16_t)be16toh(data->j1_end);
        vector->i2_start = (int16_t)be16toh(data->i2_start);
        vector->j2_start = (int16_t)be16toh(data->j2_start);
        vector->i2_end   = (int16_t)be16toh(data->i2_end);
        vector->j2_end   = (int16_t)be16toh(data->j2_end);
    }

    return 0;
}

int nexrad_packet_to_feature(nexrad_packet *packet, nexrad_geo_cartesian *radar_loc, nexrad_feature **feature) {
    if (packet == NULL || radar_loc == NULL || feature == NULL) {
        return -1;
    }

    enum nexrad_packet_type type = nexrad_packet_get_type(packet);
    int i, j;
    uint32_t id = 0;
    char name[32] = {0};
    const char *event_type = NULL;

    if (type == NEXRAD_PACKET_HAIL) {
        if (nexrad_packet_read_hail_data(packet, &i, &j, NULL, NULL, NULL) < 0) {
            return -1;
        }
        strcpy(name, "Hail");
        event_type = "HAIL";
    } else if (type == NEXRAD_PACKET_CELL) {
        char cell_id[3] = {0};
        if (nexrad_packet_read_cell_data(packet, &i, &j, cell_id, sizeof(cell_id)) < 0) {
            return -1;
        }
        strcpy(name, cell_id);
        event_type = "CELL";
        /* Parse ID: combine first two chars into a 16-bit value */
        id = (uint32_t)((((uint8_t)cell_id[0]) << 8) | (uint8_t)cell_id[1]);
    } else {
        return -1;
    }

    /* Convert i, j to polar coordinates relative to radar */
    double range_meters = sqrt((double)i*i + (double)j*j) * 250.0;
    double azimuth_degrees = atan2((double)i, (double)j) * 180.0 / M_PI;
    if (azimuth_degrees < 0) {
        azimuth_degrees += 360.0;
    }

    nexrad_geo_polar polar = { .azimuth = azimuth_degrees, .range = range_meters };
    nexrad_geo_cartesian dest;
    nexrad_geo_spheroid *spheroid = nexrad_geo_spheroid_create();
    if (spheroid == NULL) {
        return -1;
    }

    nexrad_geo_find_cartesian_dest(spheroid, radar_loc, &dest, &polar);
    nexrad_geo_spheroid_destroy(spheroid);

    /* Create geometry */
    nexrad_geo_cartesian *points = malloc(sizeof(nexrad_geo_cartesian));
    if (points == NULL) {
        return -1;
    }
    memcpy(points, &dest, sizeof(nexrad_geo_cartesian));

    nexrad_geometry *geometry = nexrad_geometry_create(NEXRAD_GEOMETRY_POINT, points, 1);
    if (geometry == NULL) {
        free(points);
        return -1;
    }

    /* Create feature */
    *feature = nexrad_feature_create(id, name, geometry, event_type, 0);
    if (*feature == NULL) {
        nexrad_geometry_destroy(geometry);
        return -1;
    }

    return 0;
}
