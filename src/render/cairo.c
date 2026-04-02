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

#include <nexrad/render/cairo.h>

#ifdef NEXRAD_HAS_CAIRO

void nexrad_cairo_draw_feature(cairo_t *cr, nexrad_projected_feature *feat) {
    if (!cr || !feat || feat->type != NEXRAD_POINT_FLOAT || feat->count == 0) {
        return;
    }

    switch (feat->feature->geometry->type) {
        case NEXRAD_GEOMETRY_POINT:
            for (size_t i = 0; i < feat->count; i++) {
                if (feat->points[i].visible) {
                    cairo_new_sub_path(cr);
                    cairo_arc(cr, feat->points[i].d.f32.x, feat->points[i].d.f32.y, 2.0, 0, 2 * 3.14159265);
                }
            }
            break;

        case NEXRAD_GEOMETRY_LINESTRING:
            for (size_t i = 0; i < feat->count; i++) {
                if (i == 0) {
                    cairo_move_to(cr, feat->points[i].d.f32.x, feat->points[i].d.f32.y);
                } else {
                    cairo_line_to(cr, feat->points[i].d.f32.x, feat->points[i].d.f32.y);
                }
            }
            break;

        case NEXRAD_GEOMETRY_POLYGON:
            for (size_t i = 0; i < feat->count; i++) {
                if (i == 0) {
                    cairo_move_to(cr, feat->points[i].d.f32.x, feat->points[i].d.f32.y);
                } else {
                    cairo_line_to(cr, feat->points[i].d.f32.x, feat->points[i].d.f32.y);
                }
            }
            cairo_close_path(cr);
            break;
    }
}

void nexrad_cairo_draw_list(cairo_t *cr, nexrad_projected_feature_list *list) {
    if (!cr || !list) {
        return;
    }

    for (size_t i = 0; i < list->count; i++) {
        nexrad_cairo_draw_feature(cr, list->features[i]);
    }
}

#endif /* NEXRAD_HAS_CAIRO */
