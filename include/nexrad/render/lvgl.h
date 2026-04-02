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

#ifndef _NEXRAD_RENDER_LVGL_H
#define _NEXRAD_RENDER_LVGL_H

#include <nexrad/feature.h>

#ifdef NEXRAD_HAS_LVGL
#include <lvgl.h>

/**
 * @file nexrad/render/lvgl.h
 * @brief LVGL rendering adapter for libnexrad features.
 * @author Bryce Lanham
 */

/**
 * @brief Draw a projected feature onto an LVGL canvas.
 * 
 * @param canvas LVGL canvas object.
 * @param feat Projected feature (should be NEXRAD_POINT_INT16 or NEXRAD_POINT_FIXED_16_16).
 * @param dsc LVGL line/polygon draw descriptor.
 */
void nexrad_lv_canvas_draw_feature(lv_obj_t *canvas, nexrad_projected_feature *feat, const lv_draw_line_dsc_t *dsc);

/**
 * @brief Add a projected feature to an LVGL vector path.
 * 
 * Only applicable if LV_USE_VECTOR_GRAPHIC is enabled.
 * 
 * @param path LVGL vector path.
 * @param feat Projected feature.
 */
#if LV_USE_VECTOR_GRAPHIC
void nexrad_lv_vector_add_feature(lv_vector_path_t *path, nexrad_projected_feature *feat);
#endif

#endif /* NEXRAD_HAS_LVGL */
#endif /* _NEXRAD_RENDER_LVGL_H */
