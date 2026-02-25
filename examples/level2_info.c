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
#include "../src/util.h"

int main(int argc, char **argv) {
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

    int count = 0;
    while (nexrad_message_next_level2_record(message, &header, &data, &size) > 0) {
        if (header->type == 31) {
            nexrad_level2_data_header *dh = nexrad_level2_get_data_header(data, size);
            if (dh) {
                nexrad_level2_radial_data *rd = nexrad_level2_get_block(dh, "RAD");
                if (rd) {
                    if (count % 100 == 0) {
                        printf("Radial %d: Azimuth %.2f, Elevation %.2f, Bins %d\n",
                            count, dh->azimuth_angle, dh->elevation_angle, (int)be16toh(rd->bin_count));
                    }
                }
            }
        }
        count++;
    }

    printf("Processed %d records\n", count);

    nexrad_message_close(message);
    return 0;
}
