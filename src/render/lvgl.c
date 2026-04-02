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

#include <nexrad/render/lvgl.h>
#include <stdlib.h>

#ifdef NEXRAD_HAS_LVGL

static void _to_lv_point(const nexrad_render_point *p, nexrad_point_type type, lv_point_precise_t *out) {
    switch (type) {
        case NEXRAD_POINT_INT16:
            out->x = p->d.i16.x;
            out->y = p->d.i16.y;
            break;
        case NEXRAD_POINT_FLOAT:
            out->x = (lv_value_precise_t)p->d.f32.x;
            out->y = (lv_value_precise_t)p->d.f32.y;
            break;
        case NEXRAD_POINT_FIXED_16_16:
            /* Convert 16.16 to float or int depending on lv_value_precise_t */
#if LV_USE_FLOAT
            out->x = (lv_value_precise_t)p->d.q16.x / 65536.0f;
            out->y = (lv_value_precise_t)p->d.q16.y / 65536.0f;
#else
            /* If LVGL uses integers for precision, it might expect a different scale.
             * Standard LVGL 9 integer precision is often 1/256 pixel (8 bits). 
             * Adjusting from 16-bit fixed to 8-bit fixed. */
            out->x = (lv_value_precise_t)(p->d.q16.x >> 8);
            out->y = (lv_value_precise_t)(p->d.q16.y >> 8);
#endif
            break;
    }
}

void nexrad_lv_canvas_draw_feature(lv_obj_t *canvas, nexrad_projected_feature *feat, const lv_draw_line_dsc_t *dsc) {
    if (!canvas || !feat || feat->count == 0) return;

    /* LVGL canvas drawing requires a layer context in v9+ or direct buffer access.
     * This implementation assumes a simplified wrapper around lv_canvas_draw_line or similar. */
    
    lv_point_precise_t *points = malloc(sizeof(lv_point_precise_t) * feat->count);
    if (!points) return;

    for (size_t i = 0; i < feat->count; i++) {
        _to_lv_point(&feat->points[i], feat->type, &points[i]);
    }

    /* Note: Actual LVGL drawing call depends on LVGL version and target object.
     * This serves as a template for bridging libnexrad points to LVGL. */

    free(points);
}

#if LV_USE_VECTOR_GRAPHIC
void nexrad_lv_vector_add_feature(lv_vector_path_t *path, nexrad_projected_feature *feat) {
    if (!path || !feat || feat->count == 0) return;

    for (size_t i = 0; i < feat->count; i++) {
        lv_point_precise_t p;
        _to_lv_point(&feat->points[i], feat->type, &p);
        
        if (i == 0) {
            lv_vector_path_move_to(path, p.x, p.y);
        } else {
            lv_vector_path_line_to(path, p.x, p.y);
        }
    }

    if (feat->feature->geometry->type == NEXRAD_GEOMETRY_POLYGON) {
        lv_vector_path_close(path);
    }
}
#endif

#endif /* NEXRAD_HAS_LVGL */
