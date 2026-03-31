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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <nexrad/geo.h>

int main(int argc, char **argv) {
    nexrad_geo_projection *proj;
    uint16_t width, height;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <projection file>\n", argv[0]);
        exit(1);
    }

    if ((proj = nexrad_geo_projection_open(argv[1])) == NULL) {
        perror("nexrad_geo_projection_open()");
        exit(1);
    }

    if (nexrad_geo_projection_read_dimensions(proj, &width, &height) < 0) {
        perror("nexrad_geo_projection_read_dimensions()");
        exit(1);
    }

    printf("Projection: %s (%ux%u)\n", argv[1], width, height);

    uint16_t test_points[][2] = {
        {0, 0},
        {width / 2, height / 2},
        {width - 1, height - 1},
        {100, 100},
        {width / 4, height / 4},
        {3 * width / 4, 3 * height / 4}
    };

    int success = 1;
    for (size_t i = 0; i < sizeof(test_points) / sizeof(test_points[0]); i++) {
        uint16_t tx = test_points[i][0];
        uint16_t ty = test_points[i][1];

        if (tx >= width || ty >= height) continue;

        nexrad_geo_cartesian latlon;
        if (nexrad_geo_projection_find_cartesian_point(proj, tx, ty, &latlon) < 0) {
            fprintf(stderr, "Failed to find cartesian point for (%u, %u)\n", tx, ty);
            success = 0;
            continue;
        }

        int16_t rx, ry;
        if (nexrad_geo_projection_latlon_to_pixel(proj, latlon.lat, latlon.lon, &rx, &ry) < 0) {
            fprintf(stderr, "Failed to map lat/lon (%.6f, %.6f) back to pixel for original (%u, %u)\n",
                latlon.lat, latlon.lon, tx, ty);
            success = 0;
            continue;
        }

        printf("Point (%u, %u) -> (%.6f, %.6f) -> (%d, %d)", tx, ty, latlon.lat, latlon.lon, rx, ry);

        if (tx == rx && ty == ry) {
            printf(" [OK]\n");
        } else {
            printf(" [FAIL]\n");
            success = 0;
        }
    }

    nexrad_geo_projection_close(proj);

    if (success) {
        printf("\nRound trip successful!\n");
        return 0;
    } else {
        printf("\nRound trip FAILED!\n");
        return 1;
    }
}
