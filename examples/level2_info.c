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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <nexrad/message.h>
#include <nexrad/level2.h>
#include <nexrad/geo.h>
#include <nexrad/color.h>
#include "../src/util.h"

int main(int argc, char **argv) {
    printf("DEBUG: main start\n");
    nexrad_message *message;
    nexrad_level2_message_header *header;
    void *data;
    size_t size;
    char station[10];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <level2_file>\n", argv[0]);
        return 1;
    }

    if ((message = nexrad_message_open(argv[1])) == NULL) {
        perror("nexrad_message_open()");
        return 1;
    }

    if (nexrad_message_get_level(message) != NEXRAD_LEVEL_2) {
        fprintf(stderr, "Error: file is not Level 2\n");
        nexrad_message_close(message);
        return 1;
    }

    nexrad_message_read_station(message, station, sizeof(station));
    printf("Station: %s\n", station);

    double lat, lon, alt;
    nexrad_geo_cartesian radar_pos = {0};
    if (nexrad_message_read_station_location(message, &lat, &lon, &alt) == 0) {
        printf("Location: %.5f, %.5f (Alt: %.1fm)\n", lat, lon, alt);
        radar_pos.lat = lat;
        radar_pos.lon = lon;
    } else {
        printf("Location: Unknown\n");
    }

    nexrad_geo_spheroid *spheroid = nexrad_geo_spheroid_create();

    int count = 0;
    int ret;
    while ((ret = nexrad_message_next_level2_record(message, &header, &data, &size)) > 0) {
        if (header->type == 31) {
            nexrad_level2_data_header *dh = nexrad_level2_get_data_header(data, size);
            if (dh) {
                nexrad_level2_radial_data *rd = nexrad_level2_get_block(dh, "RAD");
                if (rd) {
                    if (count % 100 == 0) {
                        float az = nexrad_bswap_float(dh->azimuth_angle);
                        float el = nexrad_bswap_float(dh->elevation_angle);
                        uint32_t ms = be32toh(dh->collection_time);
                        printf("Radial %d (Type 31): Time %02d:%02d:%02d.%03d, Azimuth %.2f, Elevation %.2f, Bins %d\n",
                            count, (ms / 3600000), (ms / 60000) % 60, (ms / 1000) % 60, ms % 1000, az, el, (int)be16toh(rd->bin_count));
                        
                        nexrad_level2_moment_data *ref = nexrad_level2_get_block(dh, "REF");
                        if (ref) {
                            float start_km, end_km;
                            nexrad_level2_get_moment_range(ref, 0, &start_km, &end_km);
                            float center_km = (start_km + end_km) / 2.0f;
                            
                            nexrad_geo_cartesian target;
                            double target_alt;
                            if (radar_pos.lat != 0 && spheroid) {
                                nexrad_level2_get_bin_cartesian(spheroid, &radar_pos, alt, az, el, center_km, &target, &target_alt);
                                printf("  Bin 0 pos: %.5f, %.5f (Alt %.1fm) Dist: %.3fkm\n", 
                                    target.lat, target.lon, target_alt, center_km);
                            }
                            
                            printf("  Reflectivity first 5 bins (count=%d): ", be16toh(ref->bin_count));
                            for (int i = 0; i < 5 && i < be16toh(ref->bin_count); i++) {
                                float val = nexrad_level2_decode_moment(ref, i);
                                if (val == NEXRAD_LEVEL2_NO_DATA) {
                                    printf(" ND  ");
                                } else if (val == NEXRAD_LEVEL2_RANGE_FOLDED) {
                                    printf(" RF  ");
                                } else {
                                    printf("%.1f ", val);
                                }
                            }
                            printf("\n");
                        }
                    }
                }
            }
            count++;
        } else if (header->type == 1) {
            nexrad_level2_message_type1 *t1 = (nexrad_level2_message_type1 *)data;
            if (size >= sizeof(nexrad_level2_message_type1)) {
                if (count % 100 == 0) {
                    float true_az = (float)be16toh(t1->azimuth_angle) / 8.0f * (180.0f / 4096.0f);
                    float true_el = (float)be16toh(t1->elevation_angle) / 8.0f * (180.0f / 4096.0f);
                    printf("Radial %d (Type 1): Azimuth %.2f, Elevation %.2f\n", count, true_az, true_el);
                }
            }
            count++;
        }
    }

    printf("Processed %d records (last ret = %d)\n", count, ret);

    /* Test Rasterization */
    printf("Starting rasterization test...\n");
    nexrad_message_reset_level2(message);
    
    nexrad_color_table *table = nexrad_color_table_load("colors/reflectivity.clut");
    if (!table) {
        fprintf(stderr, "Failed to load color table\n");
    } else {
        uint16_t rays = 720;
        uint16_t bins = 1840;
        uint16_t rb_meters = 250;
        
        nexrad_geo_projection *proj = nexrad_geo_projection_create_equirect("test.proj", spheroid, &radar_pos, bins, rb_meters, rays, 0.01);
        if (proj) {
            nexrad_image *image = nexrad_level2_create_projected_image(message, "REF", 1, table, proj, 2.0, 66.0);
            if (image) {
                nexrad_image_save_png(image, "output.png");
                printf("Saved rasterized image to output.png\n");
                nexrad_image_destroy(image);
            } else {
                fprintf(stderr, "Failed to create projected image\n");
            }
            nexrad_geo_projection_close(proj);
        } else {
            fprintf(stderr, "Failed to create projection\n");
        }
        nexrad_color_table_destroy(table);
    }

    nexrad_message_close(message);
    nexrad_geo_spheroid_destroy(spheroid);
    return 0;
}
