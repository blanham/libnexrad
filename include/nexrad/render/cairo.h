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

#ifndef _NEXRAD_RENDER_CAIRO_H
#define _NEXRAD_RENDER_CAIRO_H

#include <nexrad/feature.h>

#ifdef NEXRAD_HAS_CAIRO
#include <cairo.h>

/**
 * @file nexrad/render/cairo.h
 * @brief Cairo rendering adapter for libnexrad features.
 * @author Bryce Lanham
 */

/**
 * @brief Add a projected feature to a Cairo path.
 * 
 * This function adds the geometry of a projected feature to the current 
 * Cairo path. It does not perform a stroke or fill operation.
 * 
 * @param cr Cairo context.
 * @param feat Projected feature (must be NEXRAD_POINT_FLOAT).
 */
void nexrad_cairo_draw_feature(cairo_t *cr, nexrad_projected_feature *feat);

/**
 * @brief Add all features in a list to a Cairo path.
 * @param cr Cairo context.
 * @param list Projected feature list (must be NEXRAD_POINT_FLOAT).
 */
void nexrad_cairo_draw_list(cairo_t *cr, nexrad_projected_feature_list *list);

#endif /* NEXRAD_HAS_CAIRO */
#endif /* _NEXRAD_RENDER_CAIRO_H */
