/*
 * Copyright (c) 2016-2026 Bryce Lanham. Distributed under the
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

#ifndef _NEXRAD_FEATURE_H
#define _NEXRAD_FEATURE_H

#include <stdint.h>
#include <stddef.h>
#include <nexrad/geo.h>

/**
 * @file nexrad/feature.h
 * @brief Generic geographic feature API for meteorological data.
 * @author Bryce Lanham
 */

/**
 * @brief Types of geometries supported by the feature API.
 */
typedef enum _nexrad_geometry_type {
    NEXRAD_GEOMETRY_POINT,      /**< A single geographic point. */
    NEXRAD_GEOMETRY_LINESTRING, /**< A series of connected geographic points. */
    NEXRAD_GEOMETRY_POLYGON     /**< A closed series of points forming a polygon. */
} nexrad_geometry_type;

/**
 * @brief A geographic geometry container.
 */
typedef struct _nexrad_geometry {
    nexrad_geometry_type type;    /**< Type of geometry. */
    nexrad_geo_cartesian *points; /**< Array of geographic coordinates (lat/lon). */
    size_t count;                 /**< Number of points in the array. */
} nexrad_geometry;

/**
 * @brief A single meteorological feature.
 */
typedef struct _nexrad_feature {
    uint32_t id;               /**< Unique feature ID. */
    char *name;                /**< Human-readable name (e.g., "Slight Risk"). */
    nexrad_geometry *geometry; /**< Geographic geometry. */
    char *event_type;          /**< Event type string (e.g., "HAIL", "TVS"). */
    uint32_t timestamp;        /**< Feature timestamp. */
} nexrad_feature;

/**
 * @brief A list of meteorological features.
 */
typedef struct _nexrad_feature_list {
    nexrad_feature **features; /**< Array of feature pointers. */
    size_t count;              /**< Number of features in the list. */
    size_t capacity;           /**< Internal capacity of the list. */
} nexrad_feature_list;

/**
 * @brief A feature projected into screen coordinates.
 */
typedef struct _nexrad_projected_feature {
    nexrad_feature *feature;         /**< Reference to original feature. */
    nexrad_geo_screen_point *points; /**< Array of screen coordinates. */
    size_t count;                    /**< Number of points. */
} nexrad_projected_feature;

/**
 * @brief A list of features projected into screen coordinates.
 */
typedef struct _nexrad_projected_feature_list {
    nexrad_projected_feature **features; /**< Array of projected features. */
    size_t count;                        /**< Number of features in the list. */
} nexrad_projected_feature_list;

/**
 * @brief Create a new empty feature list.
 * @return A new feature list object, or NULL on failure.
 */
nexrad_feature_list *nexrad_feature_list_create();

/**
 * @brief Destroy a feature list and all contained features.
 * @param list Feature list to destroy.
 */
void nexrad_feature_list_destroy(nexrad_feature_list *list);

/**
 * @brief Add a feature to a feature list.
 * @param list Feature list to modify.
 * @param feature Feature to add.
 * @return 0 on success, -1 on failure.
 */
int nexrad_feature_list_add(nexrad_feature_list *list, nexrad_feature *feature);

/**
 * @brief Destroy a projected feature list.
 * @param list Projected feature list to destroy.
 */
void nexrad_projected_feature_list_destroy(nexrad_projected_feature_list *list);

/**
 * @brief Create a new geometry object.
 * @param type Type of geometry.
 * @param points Array of geographic coordinates to copy.
 * @param count Number of points in the array.
 * @return A new geometry object, or NULL on failure.
 */
nexrad_geometry *nexrad_geometry_create(nexrad_geometry_type type, nexrad_geo_cartesian *points, size_t count);

/**
 * @brief Destroy a geometry object.
 * @param geometry Geometry to destroy.
 */
void nexrad_geometry_destroy(nexrad_geometry *geometry);

/**
 * @brief Create a new feature object.
 * @param id Feature ID.
 * @param name Feature name (copied).
 * @param geometry Geometry object (ownership transferred).
 * @param event_type Event type string (copied).
 * @param timestamp Feature timestamp.
 * @return A new feature object, or NULL on failure.
 */
nexrad_feature *nexrad_feature_create(uint32_t id, const char *name, nexrad_geometry *geometry, const char *event_type, uint32_t timestamp);

/**
 * @brief Destroy a feature object.
 * @param feature Feature to destroy.
 */
void nexrad_feature_destroy(nexrad_feature *feature);

/**
 * @brief Parse a GeoJSON string into a feature list.
 * @param json_data Raw GeoJSON string.
 * @return A new feature list containing parsed features, or NULL on failure.
 */
nexrad_feature_list *nexrad_feature_list_from_geojson(const char *json_data);

#endif /* _NEXRAD_FEATURE_H */
